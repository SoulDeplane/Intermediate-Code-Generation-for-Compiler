from flask import Flask, request, jsonify
import time
import os
import subprocess
import re

app = Flask(__name__, template_folder='.')

def extract_section(text, start_marker, end_marker):
    try:
        start_idx = text.find(start_marker)
        if start_idx == -1:
            return ""
        
        content_start = start_idx + len(start_marker)
        if end_marker:
            end_idx = text.find(end_marker, content_start)
            if end_idx != -1:
                return text[content_start:end_idx].strip()
        
        return text[content_start:].strip()
    except Exception:
        return ""

def filter_diagnostics(stderr_text):
    if not stderr_text:
        return ""
    lines = stderr_text.splitlines()
    # Filter out the specific 'main' unused warning
    filtered_lines = [
        line for line in lines 
        if "Variable 'main' is declared but never used" not in line
    ]
    return "\n".join(filtered_lines)

@app.route('/')
def index():
    return open('index.html').read()

@app.route('/compile', methods=['POST'])
def compile_code():
    data = request.get_json()
    source_code = data.get('code', '')
    if not source_code.strip():
        return jsonify({"stderr": "Error: No code provided."}), 400
    
    try:
        with open("temp_source.c", "w") as f:
            f.write(source_code)

        process = subprocess.run(['analyzer.exe'], input=source_code, text=True, capture_output=True, timeout=5)
        stdout = process.stdout
        stderr = process.stderr
        initial_warnings = stderr

        errors_extracted = []
        for line in stderr.splitlines():
            if "Missing semicolon" in line and "at line " in line:
                match = re.search(r"at line (\d+)", line)
                if match:
                    errors_extracted.append(f"MISSING_SEMICOLON {match.group(1)}")

        has_corrections = False
        corrected_code = ""
        stdout_final = stdout
        stderr_final = stderr

        if errors_extracted:
            with open("errors.txt", "w") as f:
                f.write("\n".join(errors_extracted) + "\n")

            subprocess.run(['code_correc.exe', 'temp_source.c', 'errors.txt', 'corrected_source.c'], capture_output=True, text=True, timeout=5)

            if os.path.exists("corrected_source.c"):
                with open("corrected_source.c", "r") as f:
                    corrected_code = f.read()
                
                # Only mark as corrected if it's actually different from original (ignoring line endings)
                if corrected_code.replace('\r', '').strip() != source_code.replace('\r', '').strip():
                    has_corrections = True
                    process_corrected = subprocess.run(['analyzer.exe'], input=corrected_code, text=True, capture_output=True, timeout=5)
                    stdout_final = process_corrected.stdout
                    stderr_final = process_corrected.stderr
                else:
                    corrected_code = ""

        # Extract Sections from the FINAL run
        lex_tokens = extract_section(stdout_final, "--- Lexical Tokens ---", "---")
        parser_tree = extract_section(stdout_final, "--- Abstract Syntax Tree ---", "---")
        symbol_table = extract_section(stdout_final, "--- Symbol Table ---", None)
        
        tac = ""
        if os.path.exists("icg.txt"):
            # Run Optimizer
            subprocess.run(['python', 'optimiser.py', 'icg.txt'], capture_output=True, text=True, timeout=5)
            output_file = "optimized_icg.txt" if os.path.exists("optimized_icg.txt") else "icg.txt"
            
            with open(output_file, "r") as f:
                tac = f.read()
            
            if os.path.exists("icg.txt"): os.remove("icg.txt")
            if os.path.exists("optimized_icg.txt"): os.remove("optimized_icg.txt")
        else:
            tac = "No TAC generated."

        return jsonify({
            "tac": tac,
            "lex_tokens": lex_tokens,
            "parser_tree": parser_tree,
            "symbol_table": symbol_table,
            "corrected_code": corrected_code,
            "has_corrections": has_corrections,
            "initial_stderr": filter_diagnostics(initial_warnings),
            "final_stderr": filter_diagnostics(stderr_final)
        })
        
    except Exception as e:
        return jsonify({"stderr": str(e)}), 500

if __name__ == '__main__':
    print("Server running at http://127.0.0.1:5000")
    app.run(debug=True, port=5000, use_reloader=False)
