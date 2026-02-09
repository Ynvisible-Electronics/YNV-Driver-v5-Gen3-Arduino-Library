
# YNV_Driver_v5_Gen3 Arduino Library  
### Driver library for Ynvisible Gen3 Electrochromic Displays using the Ynvisible Driver v5

![Release Version](https://img.shields.io/github/release/Ynvisible-Electronics/YNV-Driver-v5-Gen3-Arduino-Library.svg)

---

## ⚠️ Important Notes

- This library is **only for Ynvisible Gen3 displays**.  
- For Gen1/Gen2 displays, use the legacy driver:  
  https://github.com/Ynvisible-Electronics/YNV-Driver-v5-Arduino-Library  
- **Signage Kit support has been removed** until the product is officially released.

---

# 📦 Overview

This library enables full control of **Ynvisible Gen3 electrochromic displays** using the **Driver v5** evaluation board.

It includes:

### ✔ Core Gen3 ECD Driver (`YNV_ECD`)
- Color and Bleach transitions  
- Open‑circuit potential (OCP) sampling  
- Automatic refresh engine  
- Safe CE driving (DAC‑based virtual ground)  
- Accurate LSB-based amplitude logic  

### ✔ Evaluation Kit Helpers
- Ready-to-use control functions for:
  - 7‑segment (dot)
  - 15‑segment (negative & dot)
  - 3‑bars and 7‑bars displays

### ✔ Driver v5 Board Helpers
- LED animations  
- Startup sequences  
- Simple debug indicators  

---

# 📁 Repository Structure

```
YNV-Driver-v5-Gen3-Arduino-Library/
│
├── src/
│   ├── YnvisibleECD.cpp
│   ├── YnvisibleECD.h
│   ├── YnvisibleDriverV5.cpp
│   ├── YnvisibleDriverV5.h
│   ├── YnvisibleEvaluationKit.cpp
│   └── YnvisibleEvaluationKit.h
│
├── examples/
│   └── EvaluationKit/
│
├── keywords.txt
├── CHANGELOG.md
└── library.properties
```

---

# 🔧 Installation (Arduino IDE)

1. Open **Arduino IDE**  
2. Go to: Tools → Manage Libraries…  
3. Search for **YNV_Driver_v5_Gen3**  
4. Install the library  

Done! 🎉

---

# 🖥 Getting Started

### Include the driver:

```cpp
#include <YnvisibleECD.h>
#include <YnvisibleEvaluationKit.h>
```

### Initialize the Evaluation Kit:

```cpp
evaluationKitInit();
display7SegDotRun(5, false);
```

### Basic example:

```cpp
int myPins[] = {2, 3, 4};
YNV_ECD display(3, myPins);

display.begin();                     // Color all → Bleach all
display.setSegmentState(0, true);    // ON
display.executeDisplay();

delay(2000);

display.setSegmentState(0, false);   // OFF
display.executeDisplay();
```

More examples:  
**File → Examples → YNV_Driver_v5_Gen3 → EvaluationKit**

---

# 📚 Supported Hardware

### ✔ Driver board
- Ynvisible Driver v5

### ✔ Displays (Gen3)
- 7‑Segment (w/ dot)  
- 15‑Segment (negative)  
- 15‑Segment (middle dot)  
- Single segment  
- 3‑Bar, 7‑Bar displays  

---

# 🛠 Version History

### **v1.1.0 – Major Driving Engine Update**
- Full documentation overhaul (headers, Doxygen, inline comments)  
- New organization of `.h` / `.cpp` files  
- Fixed Evaluation Kit configuration bugs  
- Improved safety: pointer protection, index bounds checks  
- Revamped electrochromic driving engine**
- Removed Signage Kit from the library (until launch)  
- No API-breaking changes  

### **v1.0.0**
- Initial release for Gen3 electrochromic displays  
- Includes Evaluation Kit example  
- Compatible with Driver v5 hardware  

---

# 📘 Documentation

Product documentation:  
https://www.ynvisible.com/shop  

---

# 🧩 Support

For questions or commercial support:  
📧 **sales@ynvisible.com**
