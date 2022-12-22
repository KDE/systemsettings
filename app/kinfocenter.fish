set --local commands -h --help --list --args --help-all -v --version --author --license --desktopfile

complete --command kinfocenter --no-files

set --local arguments (kinfocenter --list | awk 'NR>1{print $1}')

set --local descriptions (kinfocenter --list | awk 'NR>1{ print substr($0, index($0,$3)) }')

set --local argcount (seq 1 (count $arguments))

for i in $argcount
    complete --command kinfocenter --condition "not __fish_seen_subcommand_from $commands" --arguments "$arguments[$i]" --description "$descriptions[$i]"
end
