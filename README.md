# YNV_Driver_v5_Gen3 Arduino Library

Library for driving **Ynvisible Gen3 electrochromic displays** using the **Ynvisible Driver v5** board.

> ⚠️ **NOTE:**  
> This library is for **Gen3** displays only.  
> For legacy (Gen1/Gen2) displays, use the original library:  
> https://github.com/Ynvisible-Electronics/YNV-Driver-v5-Arduino-Library

---

## 🆕 Latest Release
[![Release Version](https://img.shields.io/github/release/Ynvisible-Electronics/YNV-Driver-v5-Gen3-Arduino-Library.svg)](https://github.com/Ynvisible-Electronics/YNV-Driver-v5-Gen3-Arduino-Library/releases/latest)

---

## Contents of this Repository

This repository contains the necessary libraries to work with the Driver v5 and its Evaluation and Signage Kits, using Arduino IDE.

* `YnvisibleDriverV5.cpp` contains code specific to the Driver v5 board - particularly LED management
* `YnvisibleECD.cpp` contains the `YNV_ECD` class which is used to drive Ynvisible's Electrochromic Displays using the FPC connector present in the Driver v5 board
* `YnvisibleEvaluationKit.cpp` has specific code to run the [Evaluation Kit](https://www.ynvisible.com/shop#shop), together with the `EvaluationKit.ino` Sketch
* `YnvisibleSignageKit.cpp` is used to communicate with Ynvisible's [Signage Module Kit](https://www.ynvisible.com/shop#shop) (coming soon)
* `EvaluationKit.ino` is an Arduino example Sketch used to drive the displays of the Evaluation Kit

---

## 📦 Installation

### Install from Arduino IDE (recommended)
1. Open **Arduino IDE**  
2. Go to: Tools → Manage Libraries…
3. Search for: YNV_Driver_v5_Gen3
4. Click **Install**

The library is now ready to use.

---

## 🖥 Example Sketch

After installing the library, you can find ready‑to‑use examples in:
File → Examples → YNV_Driver_v5_Gen3 → EvaluationKit

The example included is compatible with the **Ynvisible Evaluation Kit (Gen3)** available in the Ynvisible shop.

---

## 📁 Contents of this Repository

This repository contains the necessary files to drive Ynvisible Gen3 displays using the Driver v5 platform:

- `src/YnvisibleECD.cpp` – Core Gen3 display driver logic  
- `src/YnvisibleECD.h` – Public API for Gen3 ECD control  
- `examples/` – Sketches demonstrating how to use the library  
- `library.properties` – Arduino Library Manager metadata  

---

## 📘 Documentation

For product details and documentation, visit:

https://www.ynvisible.com/shop

---

## 🛠 Version Log

### v1.0.0
- Initial release for Gen3 electrochromic displays  
- Included Evaluation Kit example  
- Compatible with Driver v5 hardware  

---

## 🧩 Support

For support or issues related to this library, contact us:

📧 **sales@ynvisible.com**
