from flask import Flask, request, jsonify
import os
import subprocess
import re
import json

app = Flask(__name__, template_folder='.')

DIAG_RE = re.compile(
    r"^\[(?P<sev>warning|error|fatal)\]\[(?P<phase>[A-Z]+)\]\s+"
    r"line=(?P<line>\d+)\s+col=(?P<col>\d+)\s+code=(?P<code>[A-Z_]+):\s+"
    r"(?P<msg>.*?)(?:\s+\|\s+recovery:.*)?$"
)

UNDECLARED_RE = re.compile(r"^Undeclared identifier (\S+)")
ILLEGAL_CHAR_RE = re.compile(r"^Unexpected character '(.+)'")

def humanize_one(code, msg, line):
    if code == "PARSER_ERROR":
        if msg.startswith("syntax error"):
            return None
        m = UNDECLARED_RE.match(msg)
        if m:
            return f"Line {line}: '{m.group(1)}' is not declared. Check the spelling, or declare it before this line."
        if "Missing semicolon" in msg:
            return f"Line {line}: Missing semicolon ';' — every C statement must end with one."
        if "Missing valid type specifier" in msg:
            return f"Line {line}: Declaration is missing a type. Start with 'int', 'float', 'char', or 'void'."
        if "Missing or malformed condition inside 'if'" in msg:
            return f"Line {line}: The condition inside 'if' looks wrong. Make sure it's '(condition)'."
        if "Missing or malformed condition inside 'while'" in msg:
            return f"Line {line}: The condition inside 'while' looks wrong. Make sure it's '(condition)'."
        if "Missing closing ')'" in msg:
            return f"Line {line}: Missing closing ')'."
        if "Malformed 'for' loop" in msg:
            return f"Line {line}: 'for' loop is malformed. Expected: for (init; condition; step) {{ body }}."
        if "Invalid statement" in msg:
            return f"Line {line}: Invalid statement. Check punctuation around this line."
        return f"Line {line}: {msg}"
    if code == "UNUSED_VARIABLE":
        return f"Line {line}: Warning — {msg.lower().rstrip('.')}."
    if code == "TYPE_MISMATCH_INIT" or code == "TYPE_MISMATCH_ASSIGN":
        return f"Line {line}: Type mismatch — {msg.rstrip('.').lower()}."
    if code == "IMPLICIT_TRUNCATION":
        return f"Line {line}: Warning — possible loss of precision when assigning a decimal value to an integer."
    if code == "MISSING_RETURN":
        return f"Line {line}: Function should return a value but doesn't on every path."
    if code == "ARG_COUNT_MISMATCH":
        return f"Line {line}: Wrong number of arguments to function — {msg.rstrip('.').lower()}."
    if code == "ARG_TYPE_MISMATCH":
        return f"Line {line}: Argument type mismatch — {msg.rstrip('.').lower()}."
    if code == "UNSAFE_API":
        return f"Line {line}: Warning — unsafe function call: {msg.rstrip('.').lower()}."
    if code == "LEAK_POSSIBLE":
        return f"Line {line}: Warning — possible memory leak: {msg.rstrip('.').lower()}."
    if code == "UNCLOSED_COMMENT":
        return f"Line {line}: Unterminated comment '/*' is missing its closing '*/'."
    if code == "UNCLOSED_STRING":
        return f"Line {line}: Unterminated string literal — missing closing '\"'."
    if code == "MALFORMED_CHAR_LITERAL":
        return f"Line {line}: Malformed character literal — check the quotes."
    if code == "MALFORMED_NUMBER":
        return f"Line {line}: Malformed numeric literal."
    if code == "ILLEGAL_CHAR":
        m = ILLEGAL_CHAR_RE.match(msg)
        c = m.group(1) if m else "?"
        return f"Line {line}: Unexpected character '{c}'."
    return f"Line {line}: {msg}"

def humanize_diagnostics(stderr_text):
    if not stderr_text:
        return ""
    seen = set()
    out = []
    for raw in stderr_text.splitlines():
        m = DIAG_RE.match(raw.strip())
        if not m:
            continue
        h = humanize_one(m.group("code"), m.group("msg"), int(m.group("line")))
        if h and h not in seen:
            seen.add(h)
            out.append(h)
    return "\n".join(out)

TEMP_FILES = ("temp_source.c", "errors.txt", "corrected_source.c",
              "icg.txt", "optimized_icg.txt")

def remove_temp_files():
    for name in TEMP_FILES:
        try:
            os.remove(name)
        except FileNotFoundError:
            pass

def extract_section(text, start_marker, end_marker):
    start_idx = text.find(start_marker)
    if start_idx == -1:
        return ""
    content_start = start_idx + len(start_marker)
    if end_marker:
        end_idx = text.find("\n" + end_marker, content_start)
        if end_idx != -1:
            return text[content_start:end_idx].strip()
    return text[content_start:].strip()

def extract_corrector_rows(stderr_text):
    seen_semi = set()
    seen_undecl = set()
    rows = []
    for line in stderr_text.splitlines():
        m = DIAG_RE.match(line.strip())
        if not m:
            continue
        code = m.group("code")
        msg = m.group("msg")
        ln = int(m.group("line"))
        if code == "PARSER_ERROR":
            if "Missing semicolon" in msg and ln not in seen_semi:
                seen_semi.add(ln)
                rows.append(f"MISSING_SEMICOLON {ln}")
                continue
            mu = UNDECLARED_RE.match(msg)
            if mu:
                name = mu.group(1)
                if name not in {"include", "stdio", "stdlib", "string", "math",
                                "ctype", "time"} and name not in seen_undecl:
                    seen_undecl.add(name)
                    rows.append(f"UNDECLARED_VARIABLE {ln} {name}")
    return rows

@app.route('/')
def index():
    return open('index.html').read()

@app.route('/compile', methods=['POST'])
def compile_code():
    data = request.get_json()
    source_code = data.get('code', '')
    if not source_code.strip():
        return jsonify({"stderr": "Error: No code provided."}), 400

    remove_temp_files()
    try:
        with open("temp_source.c", "w", newline='', encoding='utf-8') as f:
            f.write(source_code)

        first = subprocess.run(['analyzer.exe'], input=source_code,
                               capture_output=True, timeout=5,
                               encoding='utf-8', errors='replace')
        stdout, stderr = first.stdout, first.stderr
        initial_stderr = stderr

        with open("errors.txt", "w", encoding='utf-8') as f:
            rows = extract_corrector_rows(stderr)
            f.write("\n".join(rows) + ("\n" if rows else ""))

        subprocess.run(['code_correc.exe', 'temp_source.c', 'errors.txt',
                        'corrected_source.c'], capture_output=True,
                       timeout=5, encoding='utf-8', errors='replace')

        corrected_code = ""
        has_corrections = False
        stdout_final, stderr_final = stdout, stderr
        if os.path.exists("corrected_source.c"):
            with open("corrected_source.c", "r", encoding='utf-8') as f:
                corrected_code = f.read()
            if corrected_code.replace('\r', '').strip() != source_code.replace('\r', '').strip():
                has_corrections = True
                if os.path.exists("icg.txt"):
                    os.remove("icg.txt")
                second = subprocess.run(['analyzer.exe'], input=corrected_code,
                                        capture_output=True, timeout=5,
                                        encoding='utf-8', errors='replace')
                stdout_final, stderr_final = second.stdout, second.stderr
            else:
                corrected_code = ""

        def parse_json_section(marker):
            raw = extract_section(stdout_final, marker, "--- ")
            if not raw:
                return None
            try:
                return json.loads(raw)
            except json.JSONDecodeError:
                return None

        lex_tokens_text = extract_section(stdout_final, "--- Lexical Tokens ---", "--- ")
        lex_tokens_json = parse_json_section("--- Lexical Tokens JSON ---")
        parser_tree_text = extract_section(stdout_final, "--- Parser Tree ---", "--- ")
        parser_tree_json = parse_json_section("--- Parser Tree JSON ---")
        symbol_table_text = extract_section(stdout_final, "--- Symbol Table ---", "--- ")
        symbol_table_json = parse_json_section("--- Symbol Table JSON ---")

        tac = ""
        if os.path.exists("icg.txt"):
            subprocess.run(['python', 'optimiser.py', 'icg.txt'],
                           capture_output=True, timeout=5,
                           encoding='utf-8', errors='replace')
            output_file = "optimized_icg.txt" if os.path.exists("optimized_icg.txt") else "icg.txt"
            with open(output_file, "r", encoding='utf-8') as f:
                tac = f.read().strip()

        return jsonify({
            "tac": tac,
            "lex_tokens": lex_tokens_text,
            "lex_tokens_json": lex_tokens_json,
            "parser_tree": parser_tree_text,
            "parser_tree_json": parser_tree_json,
            "symbol_table": symbol_table_text,
            "symbol_table_json": symbol_table_json,
            "corrected_code": corrected_code,
            "has_corrections": has_corrections,
            "initial_stderr": humanize_diagnostics(initial_stderr),
            "final_stderr": humanize_diagnostics(stderr_final)
        })
    except Exception as e:
        return jsonify({"stderr": str(e)}), 500
    finally:
        remove_temp_files()

if __name__ == '__main__':
    print("Server running at http://127.0.0.1:5000")
    app.run(debug=True, port=5000, use_reloader=False)
