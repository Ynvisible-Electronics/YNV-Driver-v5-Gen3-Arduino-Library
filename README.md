# Ynvisible Driver V5 Arduino Library

Library for driving the Ynvisible displays with the Driver v5 board.

## Latest Release

[![Release Version](https://img.shields.io/github/release/Ynvisible-Electronics/YNV-Driver-v5-Arduino-Library.svg)](https://github.com/Ynvisible-Electronics/YNV-Driver-v5-Arduino-Library/releases/latest/)

## Installation

To install this Arduino Library, simply go to the Library Manager and search for `YNV_Driver_v5`, then select the latest version and click `Install`.

That's it! The library is now installed and ready to use!

## Using the Library

You can use our example Sketch by going to `File > Examples > Ynvisible Driver v5 Arduino Library > EvaluationKit` for a plug-and-play solution.

For a personalized solution, you can use this library to create a custom Arduino Sketch for the Driver v5. See the [Contents of this repository](https://github.com/Ynvisible-Electronics/YNV-Driver-v5-Arduino-Library/edit/main/README.md#contents-of-this-repository) section for more information.

## Contents of this Repository

This repository contains the necessary libraries to work with the Driver v5 and it's Evaluation and Signage Kits, in Arduino IDE.

* `YnvisibleDriverV5.cpp` contains code specific to the Driver v5 board - particularly LED management
* `YnvisibleECD.cpp` contains the `YNV_ECD` class which is used to drive Ynvisible's Electrochromic Displays using the FPC connector present in the Driver v5 board
* `YnvisibleEvaluationKit.cpp` has specific code to run the [Evaluation Kit](https://www.ynvisible.com/shop#shop), together with the `EvaluationKit.ino` Sketch
* `YnvisibleSignageKit.cpp` is used to communicate with Ynvisible's [Signage Module Kit](https://www.ynvisible.com/shop#shop) (coming soon)
* `EvaluationKit.ino` is an Arduino example Sketch used to drive the displays of the Evaluation Kit

## Version Log

### v1.0.X

- Initial release
