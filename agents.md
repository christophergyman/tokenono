# Agent Instructions

Any planning documents, scratch notes, or temporary files should go into
`.temp/` at the root of this project.

This project does **not** use Linear. Do not create, update, or manage Linear
tickets for work in this repository.

## Memory Allocation Policy

This project uses arena allocation for project-owned runtime memory. Agents
should not default to raw `malloc`, `calloc`, `realloc`, or `free` for game/app
memory.

Before adding an allocation, choose the lifetime:

- `permanent_arena` — data that lives for the whole app run.
- `frame_arena` — temporary data that is only valid for the current frame.
- External/library-managed memory — use the library API when ownership belongs
  to raylib or another dependency.

If the correct lifetime is unclear, ask the user which arena/lifetime to use.
Do not introduce new arena types such as `level_arena`, `asset_arena`, or
`scratch_arena` without user approval.

Never store pointers allocated from `frame_arena` beyond the current frame. Do
not reset or destroy arenas outside their established lifecycle points.

## ⚠️ DO NOT COMMIT OR PUSH UNLESS EXPLICITLY ASKED

**NEVER** commit and push code to the repository unless the user **explicitly**
instructs you to do so.

If the user asks you to modify code, only write the files. You must wait for
specific approval before running `git commit` or `git push`.

The user reviews all changes before they go to the repo.
