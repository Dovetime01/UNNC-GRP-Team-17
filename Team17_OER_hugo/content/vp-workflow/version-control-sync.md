---
title: "Version Synchronization Tools in a VP Workflow"
description: "An overview of the version control tools used in this VP project — GitHub for documentation and scripts, Perforce for UE projects, and Git LFS as a theoretical alternative — with guidance on when to use each."
weight: 40
series: ["VP Workflow Foundations"]
series_order: 4
categories: ["VP Workflow"]
tags: ["version-control", "github", "perforce", "git-lfs", "collaboration", "file-sync"]
showDate: false
---

## What This Covers

A VP team produces two kinds of files that behave very differently under version control: text-based work (scripts, documentation, configuration) and large binary work (UE projects, textures, meshes). Most version control systems are optimized for one or the other — not both. This article covers the tools used in this project, why each was chosen for its specific role, and what the tradeoffs look like in practice.

---

## Why Version Control Matters in VP

VP projects have several characteristics that make file synchronization non-trivial:

- **Large file sizes:** A single UE project can reach tens of gigabytes. Cloning or syncing that over a slow connection is a significant time cost.
- **Frequent binary changes:** Every texture save, mesh export, and level modification touches large binary files. In a system optimized for text diffs, these changes are expensive to store and compare.
- **Mixed contributor roles:** Artists, technical directors, and coordinators all modify different parts of the project — but they often need the same scene state at the same time.
- **Tight production timelines:** A version conflict or sync error during a shoot day is not a minor inconvenience — it can block an entire camera team.

The right version control strategy does not just track changes. It keeps everyone on the same working state with minimal friction.

---

## Tool 1: GitHub — Documentation, Scripts, and Shared Reports

[GitHub](https://github.com/) was chosen as the team's central hub for everything that is not a UE project: project documentation, shooting reports, Python automation scripts, configuration files, and shared resources.

The reasons for this choice:

- **Text-native:** Git is built for text files. Markdown documents, Python scripts, and YAML configs diff cleanly, merge cleanly, and store efficiently. There is no binary bloat.
- **Familiar workflow:** Branching, pull requests, and code review are well-understood by most contributors. New team members can get productive quickly.
- **Good ecosystem:** GitHub Actions can automate report generation, linter checks, and deployment of documentation sites — all useful in a production environment.
- **Cross-platform:** Git and GitHub work equally well on macOS, Windows, and Linux.

**What lives here in this project:**
- Documentation and guides (this site)
- Shooting reports and production logs
- Python scripts used for scene automation or data processing
- Configuration templates and naming standard documents

**Limitation for UE project:** GitHub is not suited for large binary files. Storing UE project files, `.uasset` files, textures, or FBX meshes directly in a Git repository causes repository bloat and slow clone times. Git LFS can extend Git's capability here — see below — but for the UE project itself, a different tool was used.

---

## Tool 2: Perforce — Unreal Engine Projects

We use [Perforce](https://www.perforce.com/) to manage our UE project. Perforce was chosen for reasons directly tied to how UE projects behave:

- **Handles large binaries efficiently:** Perforce stores binary assets as compressed deltas rather than full copies on every save. A 500 MB texture that is slightly modified does not add 500 MB to the server state — only the changed blocks are recorded.
- **Exclusive locking:** Unlike Git's merge-based workflow, Perforce allows exclusive file locks. When a technical artist is actively modifying a large mesh or texture, no one else is working on the same file simultaneously. This prevents binary merge conflicts that are difficult or impossible to resolve cleanly.
- **Workspace synchronization:** Perforce's workspace model lets each user sync only the files and folders they need, rather than cloning the entire depot. For a multi-terabyte project, this is a practical necessity.
- **Revision control with no clone cost:** Because Perforce is server-based, checking out an older revision does not require re-cloning anything. The full history is always available on the server.

**Common workflow in this project:**
1. User syncs the latest revision of the relevant level or asset folder.
2. Files are opened for edit (checked out) and locked.
3. Work is done in UE and saved.
4. User submits the change with a descriptive changelist.
5. Other team members sync to receive the update.


---

## Tool 3: Git LFS — An Alternative We Did Not Use

[Git LFS (Large File Storage)](https://git-lfs.com/) is a Git extension that moves large binary files out of the Git repository proper and stores them separately on a LFS server, while keeping text-like references in the Git history.

**How it works:**
- Large files are replaced in the Git repository with small pointer files.
- The actual binary content is stored in a separate LFS store.
- `git clone` fetches the pointer files; `git checkout` pulls the actual binary content from LFS on demand.
- For most operations, the workflow looks identical to standard Git.

**Why it was considered but not adopted:**
- Git LFS requires a LFS server (either GitHub's built-in LFS storage or a self-hosted option). The storage quotas and cost model need to be planned in advance.
- Locking in Git LFS is not as robust as Perforce's exclusive locking for binary files — it is advisory rather than enforced at the server level.
- For a project of this scale where the UE project was already moving to Perforce, adding a parallel system for LFS would have added infrastructure complexity without a clear benefit.

Git LFS remains a viable option for teams that want to keep everything in Git and are comfortable managing LFS storage policies. It is worth revisiting if the team grows or the Perforce server becomes a bottleneck.

---


## Links

- [GitHub](https://github.com/)
- [Perforce (Helix Core)](https://www.perforce.com/)
- [Git LFS](https://git-lfs.com/)
