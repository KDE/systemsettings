set --local commands -h --help --list --args --help-all -v --version --author --license --desktopfile

complete --command kcmshell5 --no-files

set --local arguments (kcmshell5 --list | awk 'NR>1{print $1}')

set --local descriptions (kcmshell5 --list | awk 'NR>1{ print substr($0, index($0,$3)) }')

set --local argcount (seq 1 (count $arguments))

for i in $argcount
    complete --command kcmshell5 --condition "not __fish_seen_subcommand_from $commands" --arguments "$arguments[$i]" --description "$descriptions[$i]"
end
