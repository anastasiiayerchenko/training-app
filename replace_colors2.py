import re

with open("frontend/css/style.css", "r") as f:
    css = f.read()

# Remaining blues: rgba(9, 26, 69, ...)
css = re.sub(r'rgba\(9,\s*26,\s*69,\s*([0-9.]+)\)', r'rgba(84, 55, 52, \1)', css)

# Old button hover backgrounds, e.g. #091a45 -> #4a302e
css = re.sub(r'#091a45', r'#4a302e', css)

# Fix floating sidebar menu active icon container
css = re.sub(r'background:\s*linear-gradient\([^)]*\)\s*!important;', r'background: #3e2726 !important;', css)

with open("frontend/css/style.css", "w") as f:
    f.write(css)
