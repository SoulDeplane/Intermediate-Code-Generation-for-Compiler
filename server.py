from flask import Flask, request, jsonify
import time
import os

app = Flask(__name__, template_folder='.')

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
        import subprocess
        import re
        with open("temp_source.c", "w") as f:
            f.write(source_code)

        process = subprocess.run(['analyzer.exe'], input=source_code, text=True, capture_output=True, timeout=5)
        stdout = process.stdout
        stderr = process.stderr

        errors_extracted = []
        for line in stderr.splitlines():
            if "Missing semicolon" in line and "at line " in line:
                match = re.search(r"at line (\d+)", line)
                if match:
                    errors_extracted.append(f"MISSING_SEMICOLON {match.group(1)}")

        with open("errors.txt", "w") as f:
            f.write("\n".join(errors_extracted) + "\n")

        subprocess.run(['code_correc.exe', 'temp_source.c', 'errors.txt', 'corrected_source.c'], capture_output=True, text=True, timeout=5)

        corrected_code = ""
        if os.path.exists("corrected_source.c"):
            with open("corrected_source.c", "r") as f:
                corrected_code = f.read()

        if corrected_code and corrected_code.strip() == source_code.strip():
            corrected_code = ""

        corrected_stdout = ""
        corrected_stderr = ""
        if corrected_code:
            process_corrected = subprocess.run(['analyzer.exe'], input=corrected_code, text=True, capture_output=True, timeout=5)
            corrected_stdout = process_corrected.stdout
            corrected_stderr = process_corrected.stderr
            stdout = corrected_stdout
            stderr = corrected_stderr

        tac = ""
        if os.path.exists("icg.txt"):
            with open("icg.txt", "r") as f:
                tac = f.read()
            os.remove("icg.txt")
        else:
            tac = "No TAC generated."

        if corrected_code:
            compilation_halted = "Compilation Halted" in corrected_stdout
            has_friendly_error = "[Friendly Compiler Notice] Error" in corrected_stderr or "syntax error" in corrected_stderr.lower()
            if compilation_halted or has_friendly_error:
                tac = "No TAC generated."
                if corrected_stderr.strip():
                    stderr = corrected_stderr
                else:
                    stderr = "Unfixable input: auto-correction could not produce valid C code."

        sym_idx = stdout.find("\n--- Symbol Table ---")
        if sym_idx != -1:
            stdout = stdout[:sym_idx].rstrip() + "\n"

        return jsonify({
            "stdout": stdout,
            "stderr": stderr,
            "tac": tac,
            "corrected_code": corrected_code
        })
    except Exception as e:
        return jsonify({"stderr": str(e)}), 500

if __name__ == '__main__':
    import logging
    try:
        from flask import cli as flask_cli
        flask_cli.show_server_banner = lambda *args, **kwargs: None
    except Exception:
        pass
    logging.getLogger("werkzeug").disabled = True
    app.logger.disabled = True
    print("Server running at http://127.0.0.1:5000")
    app.run(debug=False, port=5000, use_reloader=False)
