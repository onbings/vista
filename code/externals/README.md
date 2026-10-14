# Usage

Add here the "external" projects that need to be collected by source (i.e. through add_subdirectory)

## Adding an external

When called from this directory, the command would look something like :

```
$> git submodule add <url>
```

Once done, don't forget to add your "add_subdirectory" to the solutions you need to (e.g. <root>/code/all.cmake)

## Removing an external

First, remove your "add_subdirectory" from the solutions it was in

Then, remove the submodule

```
$> git rm <path_to_submodule>
```

As the docs note however, the .git dir of the submodule is kept around (in the modules/ directory of the main project's .git dir), "to make it possible to checkout past commits without requiring fetching from another repository".
If you nonetheless want to remove this info, manually delete the submodule's directory in .git/modules/, and remove the submodule's entry in the file .git/config.

These steps can be automated using the commands

```
rm -rf .git/modules/<path-to-submodule>, and
git config --remove-section submodule.<path-to-submodule>.
```

Credits : https://stackoverflow.com/questions/1260748/how-do-i-remove-a-submodule

# Import remark

Once you have added or modified an external, you will want others to see your changes.

By default, a simple "git pull" is not enough.

There are multiple options to work this around :

 * git pull --recurse-submodules
 * git config --global submodule.recurse true

The second option only need to be done once and then apply "git pull --recurse-submodules" when actually calling "git pull"
