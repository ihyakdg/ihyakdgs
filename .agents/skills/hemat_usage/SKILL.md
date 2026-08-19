---
name: hemat_usage
description: Membantu menghemat penggunaan token/cost API dengan menerapkan taktik interaksi yang efisien dan meminimalkan pembacaan berkas secara berlebihan.
---

# Hemat Usage Skill

This skill provides guidelines for the agent to optimize token usage, minimize context size, and avoid API cost leaks.

## Rules to Minimize Usage

1. **Targeted Reading (Strict Range Limitation)**
   - DO NOT read full files with `view_file` if they are larger than 100 lines.
   - Use `grep_search` to pinpoint target locations first.
   - Specify precise `StartLine` and `EndLine` to read only the lines you need to see.

2. **Targeted Writing/Editing**
   - Use `replace_file_content` instead of rewriting large files.
   - Provide minimal, precise `TargetContent` blocks to avoid transmitting unnecessary lines.

3. **Directory Listing and File Discovery**
   - Do not call `list_dir` recursively on massive directories.
   - Use `grep_search` with standard glob patterns (e.g. `*.cpp`, `*.h`) in `Includes` to locate files directly.

4. **Concise Agent Communication**
   - When communicating with subagents, write short, actionable, and clear prompts. Avoid boilerplate text.
   - Keep user responses clean and brief. Do not summarize code changes in the chat if they are already detailed in artifacts (like `walkthrough.md`).

5. **Subagent Control**
   - Only spawn subagents when highly concurrent or separate context research is truly required.
   - Reuse active subagents via `send_message` rather than invoking new ones when tasks are related.

6. **Context Cleaning**
   - Avoid creating redundant scratch files.
   - Avoid repeatedly reading the same files unless they have been modified.
