// Copyright Epic Games, Inc. All Rights Reserved.

#include "PointCloud/VSCalibratorGeometryPostprocessUtils.h"

namespace VSCalibratorGeometryPostprocessUtils
{
	namespace
	{
		bool TryBuildPlaneFrom3Points(const FVector& A, const FVector& B, const FVector& C, FVector& OutNormal, float& OutD)
		{
			const FVector AB = B - A;
			const FVector AC = C - A;
			FVector Normal = FVector::CrossProduct(AB, AC);
			const float LengthSquared = Normal.SizeSquared();
			if (LengthSquared <= KINDA_SMALL_NUMBER)
			{
				return false;
			}

			Normal /= FMath::Sqrt(LengthSquared);
			OutNormal = Normal;
			OutD = -FVector::DotProduct(Normal, A);
			return true;
		}

		int32 CountPlaneInliers(const TArray<FVector>& Points, const FVector& Normal, const float D, const float DistanceThreshold)
		{
			int32 InlierCount = 0;
			for (const FVector& Point : Points)
			{
				const float Distance = FMath::Abs(FVector::DotProduct(Normal, Point) + D);
				if (Distance <= DistanceThreshold)
				{
					++InlierCount;
				}
			}
			return InlierCount;
		}

		bool DetectDominantPlane(
			const TArray<FVector>& Points,
			const FVSCalibratorGeometryPostprocessConfig& Config,
			FVSCalibratorPlaneCandidate& OutPlane,
			float* OutBestInlierRatio = nullptr)
		{
			if (Points.Num() < 64)
			{
				if (OutBestInlierRatio)
				{
					*OutBestInlierRatio = 0.0f;
				}
				return false;
			}

			const int32 Iterations = FMath::Clamp(Points.Num() / 2000, 120, 600);
			FVector BestNormal = FVector::UpVector;
			float BestD = 0.0f;
			int32 BestInliers = 0;

			for (int32 Iteration = 0; Iteration < Iterations; ++Iteration)
			{
				const int32 I0 = FMath::RandRange(0, Points.Num() - 1);
				const int32 I1 = FMath::RandRange(0, Points.Num() - 1);
				const int32 I2 = FMath::RandRange(0, Points.Num() - 1);
				if (I0 == I1 || I0 == I2 || I1 == I2)
				{
					continue;
				}

				FVector CandidateNormal;
				float CandidateD = 0.0f;
				if (!TryBuildPlaneFrom3Points(Points[I0], Points[I1], Points[I2], CandidateNormal, CandidateD))
				{
					continue;
				}

				const int32 CandidateInliers = CountPlaneInliers(Points, CandidateNormal, CandidateD, Config.PlaneRansacDistanceThreshold);
				if (CandidateInliers > BestInliers)
				{
					BestInliers = CandidateInliers;
					BestNormal = CandidateNormal;
					BestD = CandidateD;
				}
			}

			const float InlierRatio = static_cast<float>(BestInliers) / static_cast<float>(Points.Num());
			if (OutBestInlierRatio)
			{
				*OutBestInlierRatio = InlierRatio;
			}

			if (BestInliers <= 0 || InlierRatio < Config.PlaneMinInlierRatio)
			{
				return false;
			}

			OutPlane.Normal = BestNormal;
			OutPlane.PlaneOffset = BestD;
			OutPlane.InlierCount = BestInliers;
			OutPlane.Confidence = InlierRatio;
			return true;
		}

		void BuildManhattanAxesFromPrimaryNormal(
			const FVector& PrimaryNormal,
			const float MaxSnapAngleDeg,
			FVSCalibratorManhattanAxes& OutAxes)
		{
			const FVector ZAxis = PrimaryNormal.GetSafeNormal(KINDA_SMALL_NUMBER, FVector::UpVector);
			const FVector Seed = (FMath::Abs(FVector::DotProduct(ZAxis, FVector::ForwardVector)) < 0.95f)
				? FVector::ForwardVector
				: FVector::RightVector;
			const FVector XAxis = FVector::CrossProduct(Seed, ZAxis).GetSafeNormal(KINDA_SMALL_NUMBER, FVector::RightVector);
			const FVector YAxis = FVector::CrossProduct(ZAxis, XAxis).GetSafeNormal(KINDA_SMALL_NUMBER, FVector::ForwardVector);

			OutAxes.bValid = true;
			OutAxes.AxisX = XAxis;
			OutAxes.AxisY = YAxis;
			OutAxes.AxisZ = ZAxis;
			OutAxes.MaxSnapAngleDeg = MaxSnapAngleDeg;
		}

		FVector ChooseClosestAxisDirection(const FVector& Normal, const FVSCalibratorManhattanAxes& Axes, float& OutAngleDeg)
		{
			const FVector UnitNormal = Normal.GetSafeNormal(KINDA_SMALL_NUMBER, FVector::UpVector);
			const FVector Candidates[6] =
			{
				Axes.AxisX,
				-Axes.AxisX,
				Axes.AxisY,
				-Axes.AxisY,
				Axes.AxisZ,
				-Axes.AxisZ
			};

			float BestDot = -1.0f;
			FVector BestAxis = Axes.AxisZ;
			for (const FVector& Candidate : Candidates)
			{
				const float Dot = FVector::DotProduct(UnitNormal, Candidate.GetSafeNormal(KINDA_SMALL_NUMBER, FVector::UpVector));
				if (Dot > BestDot)
				{
					BestDot = Dot;
					BestAxis = Candidate;
				}
			}

			OutAngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(BestDot, -1.0f, 1.0f)));
			return BestAxis.GetSafeNormal(KINDA_SMALL_NUMBER, FVector::UpVector);
		}

		bool ProjectPointsToPlaneIfEligible(
			const FVSCalibratorGeometryPostprocessConfig& Config,
			const FVSCalibratorManhattanAxes& Axes,
			const FVSCalibratorPlaneCandidate& InputPlane,
			TArray<FVector>& InOutPoints,
			FVSCalibratorPlaneCandidate& OutPlane,
			int32& OutProjectedCount,
			float& OutMeanAbsDistBefore,
			float& OutMeanAbsDistAfter,
			FString& OutNote)
		{
			OutPlane = InputPlane;
			OutProjectedCount = 0;
			OutMeanAbsDistBefore = 0.0f;
			OutMeanAbsDistAfter = 0.0f;
			OutNote = TEXT("projection skipped");

			if (!Config.bEnablePlaneProjection || !Axes.bValid)
			{
				return false;
			}

			float SnapAngleDeg = 0.0f;
			const FVector SnappedNormal = ChooseClosestAxisDirection(InputPlane.Normal, Axes, SnapAngleDeg);
			if (SnapAngleDeg > Config.MaxSnapAngleDeg)
			{
				OutNote = FString::Printf(TEXT("projection skipped: snap angle %.2fdeg > %.2fdeg"), SnapAngleDeg, Config.MaxSnapAngleDeg);
				return false;
			}

			TArray<int32> InlierIndices;
			InlierIndices.Reserve(InOutPoints.Num());
			FVector InlierCentroid = FVector::ZeroVector;
			for (int32 Index = 0; Index < InOutPoints.Num(); ++Index)
			{
				const float SignedDistance = FVector::DotProduct(InputPlane.Normal, InOutPoints[Index]) + InputPlane.PlaneOffset;
				if (FMath::Abs(SignedDistance) <= Config.ProjectionInlierDistanceThreshold)
				{
					InlierIndices.Add(Index);
					InlierCentroid += InOutPoints[Index];
					OutMeanAbsDistBefore += FMath::Abs(SignedDistance);
				}
			}

			if (InlierIndices.IsEmpty())
			{
				OutNote = TEXT("projection skipped: no projection inliers");
				return false;
			}

			InlierCentroid /= static_cast<float>(InlierIndices.Num());
			OutMeanAbsDistBefore /= static_cast<float>(InlierIndices.Num());

			const float SnappedD = -FVector::DotProduct(SnappedNormal, InlierCentroid);
			for (const int32 Index : InlierIndices)
			{
				const float Distance = FVector::DotProduct(SnappedNormal, InOutPoints[Index]) + SnappedD;
				InOutPoints[Index] = InOutPoints[Index] - Distance * SnappedNormal;
				OutMeanAbsDistAfter += FMath::Abs(FVector::DotProduct(SnappedNormal, InOutPoints[Index]) + SnappedD);
			}

			OutMeanAbsDistAfter /= static_cast<float>(InlierIndices.Num());
			OutProjectedCount = InlierIndices.Num();
			OutPlane.Normal = SnappedNormal;
			OutPlane.PlaneOffset = SnappedD;
			OutPlane.InlierCount = OutProjectedCount;
			OutPlane.Confidence = static_cast<float>(OutProjectedCount) / static_cast<float>(InOutPoints.Num());
			OutNote = FString::Printf(TEXT("projection applied: projected=%d snap=%.2fdeg"), OutProjectedCount, SnapAngleDeg);
			return true;
		}
	}

	bool RunPostDepthGeometry(
		const TArray<FVector>& InCameraPoints,
		const FVSCalibratorGeometryPostprocessConfig& Config,
		FVSCalibratorGeometryPostprocessResult& OutResult,
		FString& OutError)
	{
		OutError.Empty();
		OutResult = FVSCalibratorGeometryPostprocessResult();

		if (InCameraPoints.IsEmpty())
		{
			OutError = TEXT("Point cloud is empty.");
			return false;
		}

		OutResult.CameraPoints = InCameraPoints;

		const bool bAnyGeometryPassEnabled = Config.bEnablePlaneDetection || Config.bEnableManhattanConstraint;
		if (!bAnyGeometryPassEnabled)
		{
			OutResult.bUsedPassthrough = true;
			OutResult.StageNote = TEXT("geometry-pass disabled: passthrough");
			return true;
		}

		if (Config.bEnablePlaneDetection)
		{
			FVSCalibratorPlaneCandidate DominantPlane;
			float BestRatio = 0.0f;
			if (DetectDominantPlane(OutResult.CameraPoints, Config, DominantPlane, &BestRatio))
			{
				OutResult.Planes.Add(DominantPlane);
				OutResult.StageNote = TEXT("dominant-plane detected");
			}
			else
			{
				FVSCalibratorGeometryPostprocessConfig Relaxed = Config;
				Relaxed.PlaneRansacDistanceThreshold = FMath::Max(Config.PlaneRansacDistanceThreshold * 2.0f, 0.08f);
				Relaxed.PlaneMinInlierRatio = FMath::Clamp(Config.PlaneMinInlierRatio * 0.6f, 0.06f, 0.5f);

				float RelaxedBestRatio = 0.0f;
				if (DetectDominantPlane(OutResult.CameraPoints, Relaxed, DominantPlane, &RelaxedBestRatio))
				{
					OutResult.Planes.Add(DominantPlane);
					OutResult.StageNote = FString::Printf(
						TEXT("dominant-plane detected with relaxed thresholds (dist=%.3f minInlier=%.2f bestRatio=%.3f)"),
						Relaxed.PlaneRansacDistanceThreshold,
						Relaxed.PlaneMinInlierRatio,
						RelaxedBestRatio);
				}
				else
				{
					OutResult.StageNote = FString::Printf(
						TEXT("plane-detection failed: bestRatio=%.3f required>=%.3f (relaxedBest=%.3f relaxedRequired>=%.3f)"),
						BestRatio,
						Config.PlaneMinInlierRatio,
						RelaxedBestRatio,
						Relaxed.PlaneMinInlierRatio);
				}
			}
		}

		if (Config.bEnableManhattanConstraint)
		{
			if (!OutResult.Planes.IsEmpty())
			{
				BuildManhattanAxesFromPrimaryNormal(OutResult.Planes[0].Normal, Config.MaxSnapAngleDeg, OutResult.ManhattanAxes);
				OutResult.StageNote += TEXT("; manhattan-axes estimated");

				FVSCalibratorPlaneCandidate RefinedPlane;
				FString ProjectionNote;
				if (ProjectPointsToPlaneIfEligible(
					Config,
					OutResult.ManhattanAxes,
					OutResult.Planes[0],
					OutResult.CameraPoints,
					RefinedPlane,
					OutResult.ProjectedPointCount,
					OutResult.MeanAbsPlaneDistBefore,
					OutResult.MeanAbsPlaneDistAfter,
					ProjectionNote))
				{
					OutResult.Planes[0] = RefinedPlane;
					OutResult.bUsedPassthrough = false;
					OutResult.StageNote += TEXT("; ") + ProjectionNote;
				}
				else
				{
					OutResult.StageNote += TEXT("; ") + ProjectionNote;
				}
			}
			else
			{
				OutResult.StageNote += TEXT("; manhattan skipped (no plane)");
				OutResult.ManhattanAxes.MaxSnapAngleDeg = Config.MaxSnapAngleDeg;
			}
		}

		if (OutResult.bUsedPassthrough && OutResult.StageNote.IsEmpty())
		{
			OutResult.StageNote = TEXT("geometry pass completed without shape updates");
		}

		return true;
	}
}
