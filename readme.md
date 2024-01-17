<br />
<div align="center">
  <a href="https://github.com/MjnMixael/Etemenanki">
    <img src="Etemenanki/icon.png" alt="Logo" width="80" height="80">
  </a>

  <h3 align="center">Etemenanki</h3>

  <p align="center">
    A translation tool for FreespaceOpen!
  </p>
</div>

## About

Etemenanki is a tool meant to facilitate creating a full tstrings.tbl for games in the [FreespaceOpen engine](https://github.com/scp-fs2open/fs2open.github.com). It recursively scans a directory for valid files and uses regex to find translatable strings and string IDs. It will then replace the ID with a valid one and save the string and ID to the generated tstrings.tbl file.

Etemenanki uses [QT Designer](https://www.qt.io/product/ui-design-tools) for it's UI development.

## Usage

<img src="Preview.png">

Etemenanki will scan the provided directory for parsable files. It will then search each file with each provided Regex to try to find possible matches for translatable strings. If a match is found it will decipher the ID and check that it is both unique and not invalid*. If necessary it will provide a new unique string ID. Then the string and the ID will be saved to the output file in a proper [tstrings.tbl format](https://wiki.hard-light.net/index.php/Tstrings.tbl) for the FreepsaceOpen Engine.

Below is an explenation of the settings:

* Directory Path: The root path to the game files you want to run Etemenanki on.
* Regex List: The [Regular Expressions](https://en.wikipedia.org/wiki/Regular_expression) to be used when searching for translatable strings and IDs. Each expression also needs to specify which of the patterns in the expression are the string and the ID. This is done with index values from left to right starting with 1.
* File Formats List: In order to speed up the process, Etemenanki will only parse files with extensions listed here. All other files will be ignored.
* ID Offset: By default Etemenanki will generate new string IDs at 0, but you can provide an optional offset here instead.
* Replace Existing: By default Etemenanki will not replace strings that have an existing valid ID. Check this button if you would like to change that behavior.

When your settings are correct, click Begin and Etemenanki will begin processing your game files. The output will list any terminal errors, otherwise it will display the file it is currently parsing. A full output log can be found in %appdata%/Etemenanki/Etemenanki.log if needed.

You may Terminate the process at any time by clicking on Terminate, but the action cannot be undone! Any files that have been processed may have already been altered. It is a good idea to use this in conjunction with a versioning system like GIT or SVN which can then be used to verify Etemenanki's output.

*Invalid IDs are IDs where two strings share an ID but the strings are not identical

## Known Issues

- Etemenanki parses the files line by line, so any regex matches must be contained on a single line in their respective source files. Using \n in the regex will not work unless the source file also uses \n to directly create the new line as opposed to a return carriage with the file itself.
