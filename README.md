# pacman-declare - declarative wrapper for pacman

## Usage:
To use the script you must first create a config file for it (currently hardcoded as packages.conf). \
To create the config file you can simply `pacman -Qqe > packages.conf`. \
The config syntax looks like this:
```ini
# regualar line comments
!include foo.list # inlines the file 'foo.list'
package1          # declares package1
package2 package3 # declares package2 and package3
```
Once the config exists you can run the script:
```
./diff.sh [options]
Options:
  -r      Remove packages
  -a      Add packages
  -d      Only show diff
  -c      Show common packages
  -f      No confirm (Scary!)
  -q      Quiet mode
  -h      Display this help message
```
