# CCS code templates

Templates for new `.h` and `.c` files that follow the team's
**C File Layout Standard – One-Line Banner Style**.

| Document | Where |
| --- | --- |
| The standard (PDF, in this repo) | [`Docs/C_File_Layout_Standard_OneLine_Banner.pdf`](../C_File_Layout_Standard_OneLine_Banner.pdf) |
| The standard (live doc, always the latest) | https://claude.ai/artifact/DM38HXTCz7fPDzZkyEZaPU |

The PDF is a copy of the live doc. When the standard changes, update the live doc first,
export a new PDF, and regenerate these templates so all three match.

## Files

| File | What it is |
| --- | --- |
| `VEGA_ccs_code_templates.xml` | Import this into CCS. Holds "VEGA C header" and "VEGA C source". |
| `template.h` | Header template as plain text (for reading or copying by hand) |
| `template.c` | Source template as plain text (for reading or copying by hand) |

Keep these files here, outside the CCS project folder. CCS compiles every `.c` file in the
project, and `template.c` contains `${file_base}`, which only works inside the template.

## Use in CCS

1. **Import once:** Window → Preferences → C/C++ → Code Style → Code Templates → **Import…**,
   select `VEGA_ccs_code_templates.xml`.
2. **New file:** right-click a folder → New → **Header File** or **Source File** →
   Template: **VEGA C header** or **VEGA C source** → Finish.
3. **Fill in:** replace every `<…>` placeholder and rename `module_` / `MODULE_` to the module name.
4. **Clean up before commit:** delete every section the file does not use, together with its banner
   (the standard: an empty section has no banner).

CCS fills in `${file_name}`, `${file_base}`, `${include_guard_symbol}`, `${date}` and `${user}`
(your Windows user name; replace it with your name).
