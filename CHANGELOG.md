===========================================
 Ynvisible Driver v5 Gen3 Library – CHANGELOG
 Version: 1.1.0
 Release date: Jan 2026
===========================================


### 🚀 New in v1.1.0 (Major Driving Engine Update)

This release includes a complete overhaul of the electrochromic driving engine,
resulting in improved stability, improved refresh accuracy and much stronger
consistency across all Gen3 displays.

The optimizations lead to **more reliable coloration/bleaching**, **more stable OCP
measurements**, and significantly fewer refresh cycles required in normal use.

Key improvements:

-------------------------------------------
🔧 ELECTROCHROMIC DRIVING ENGINE IMPROVEMENTS
-------------------------------------------

• Fully revised check_refresh() logic  
  – Add half‑amplitude calculation  
  – Correct domain handling (absolute LSB vs amplitude LSB)  
  – Eliminated mismatch between LSB/V decisions  

• BLEACH refresh logic corrected and optimized  
  – CE amplitude now dynamically adapted to min OCP  
  – Prevents driving segments into unintended negative region  
  – Reduces stress on the EC material  

• COLOR refresh logic made consistent with BLEACH  
  – Clearer thresholds  
  – More reliable OCP convergence  
  – Reduced number of refresh retries

• Improved OCP sampling  
  – Stabilized CE during reads  
  – Reduced unnecessary reads  
  – More deterministic behavior

-------------------------------------------
🧩 API & ARCHITECTURE IMPROVEMENTS
-------------------------------------------

• Internal .h/.cpp file structure cleaned and standardized  
• Minor safety fixes:
  – pointer validation for p_currentDisplay  
  – strict bounds checks  
  – corrected logic bug in setNumberOfDisplays()  
  – corrected configuration bug in 15‑segment Eval Kit setup  

• Added detailed inline comments throughout the codebase  
• Added full Doxygen headers for all modules  
• Reformatted Evaluation Kit and Driver v5 modules  
• keywords.txt updated for Arduino IDE syntax highlighting  
• Signage Kit files removed (product not yet released)

-------------------------------------------
🧱 COMPATIBILITY
-------------------------------------------

• No API‑breaking changes  
• Existing sketches remain compatible  
• Improved behavior without requiring user code changes

-------------------------------------------
📦 SUMMARY
-------------------------------------------

Version 1.1.0 provides:

✓ More accurate driving  
✓ More efficient refresh  
✓ Safer CE amplitude handling  
✓ More stable electrochromic behavior  
✓ Better long‑term reliability of Gen3 displays  
✓ Cleaner and more maintainable codebase  
✓ No breaking changes for users

===========================================
 End of File
===========================================