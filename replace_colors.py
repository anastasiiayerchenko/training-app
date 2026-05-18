import re

with open("frontend/css/style.css", "r") as f:
    css = f.read()

# Replace main background gradients
css = re.sub(r'linear-gradient\(135deg,\s*#091a45\s*0%,\s*#040b1e\s*100%\)', r'linear-gradient(135deg, #22252a 0%, #15171a 100%)', css)
css = re.sub(r'linear-gradient\(180deg,\s*#183375\s*0%,\s*#091a45\s*100%\)', r'linear-gradient(180deg, #22252a 0%, #1b1d22 100%)', css)
css = re.sub(r'#091a45', r'#22252a', css)
css = re.sub(r'#183375', r'#2a2d34', css)
css = re.sub(r'#040b1e', r'#15171a', css)

# Replace translucent blue backgrounds with dark gray
css = re.sub(r'rgba\(9,\s*26,\s*69,\s*0\.45\)', r'rgba(40, 43, 48, 0.45)', css)
css = re.sub(r'rgba\(9,\s*26,\s*69,\s*0\.75\)', r'rgba(30, 32, 36, 0.75)', css)
css = re.sub(r'rgba\(9,\s*26,\s*69,\s*0\.85\)', r'rgba(26, 28, 32, 0.85)', css)
css = re.sub(r'rgba\(9,\s*26,\s*69,\s*0\.95\)', r'rgba(26, 28, 32, 0.95)', css)
css = re.sub(r'rgba\(4,\s*11,\s*30,\s*0\.7\)', r'rgba(15, 16, 18, 0.7)', css)

# Replace borders to copper red/brown
css = re.sub(r'border:\s*1px\s*solid\s*rgba\(255,\s*255,\s*255,\s*0\.15\)', r'border: 1px solid rgba(138, 85, 78, 0.6)', css)
css = re.sub(r'border:\s*1px\s*solid\s*rgba\(255,\s*255,\s*255,\s*0\.2\)', r'border: 1px solid rgba(138, 85, 78, 0.4)', css)
css = re.sub(r'border-bottom:\s*1px\s*solid\s*rgba\(255,\s*255,\s*255,\s*0\.1\)', r'border-bottom: 1px solid rgba(138, 85, 78, 0.3)', css)
css = re.sub(r'border-top:\s*1px\s*solid\s*rgba\(255,\s*255,\s*255,\s*0\.1\)', r'border-top: 1px solid rgba(138, 85, 78, 0.3)', css)
css = re.sub(r'border:\s*1px\s*solid\s*rgba\(255,\s*255,\s*255,\s*0\.08\)', r'border: 1px solid rgba(138, 85, 78, 0.2)', css)

# Fix Button .btn-glossy, .btn-create-glow, .btn-add-record
# In original style, btn-add-record was:
# background: linear-gradient(180deg, #183375 0%, #091a45 100%);
# Let's change these buttons to the solid brown color #5e3a38 (which is the color the user requested)
# We can use regex to replace the background of btn-add-record and btn-create-glow
css = re.sub(r'\.btn-add-record\s*{[^}]*}', r'.btn-add-record {\n  background: #543734 !important;\n  border: none !important;\n  border-radius: 20px;\n  padding: 14px 20px;\n  color: #ffffff;\n  font-weight: 600;\n  font-size: 1rem;\n  cursor: pointer;\n  width: 100%;\n  display: flex;\n  justify-content: center;\n  align-items: center;\n  gap: 8px;\n  transition: all 0.2s ease;\n}', css)

css = re.sub(r'\.btn-create-glow\s*{[^}]*}', r'.btn-create-glow {\n  background: #543734 !important;\n  border: none !important;\n  border-radius: 20px;\n  padding: 14px 20px;\n  color: #ffffff;\n  font-weight: 600;\n  font-size: 1rem;\n  cursor: pointer;\n  width: 100%;\n  display: flex;\n  justify-content: center;\n  align-items: center;\n  gap: 8px;\n  transition: all 0.2s ease;\n}', css)

css = re.sub(r'\.btn-glossy\s*{[^}]*}', r'.btn-glossy {\n  background: #543734 !important;\n  border: none !important;\n  border-radius: 20px;\n  padding: 14px 20px;\n  color: #ffffff;\n  font-weight: 600;\n  font-size: 1rem;\n  cursor: pointer;\n  width: 100%;\n  display: flex;\n  justify-content: center;\n  align-items: center;\n  gap: 8px;\n  transition: all 0.2s ease;\n}', css)

with open("frontend/css/style.css", "w") as f:
    f.write(css)

