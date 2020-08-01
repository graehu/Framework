#!/usr/bin/env python
import os
import sys
# sys.path.append("tools/confply/")
# import confply.log as log

# print(os.get_terminal_size())

# requires / ending
projects = [
    "src/application/rc_sample/",
    "src/application/net_physics_sample/"
]

if __name__ == "__main__":

    in_args = sys.argv[1:]
    fails = 0
    if len(in_args) != 0:
        for arg in in_args:
            project_found = False
            for project in projects:
                project_name = project.split("/")[-2]
                if project_name == arg:
                    if os.path.isfile(project+"build.py"):
                        project_found = True
                        if os.system("cd "+project+"; ./build.py") != 0:
                            fails += 1
                    else:
                        print("Couldn't find build.py under "+project)
                        
            if project_found == False:
                print("Couldn't find project: "+arg)
    else:
        for project in projects:
            if os.path.isfile(project+"build.py"):
                if os.system("cd "+project+"; ./build.py") != 0:
                    fails += 1
                    
            else:
                print("Couldn't find build.py under "+project)

    if fails == 0:
        print("all projects compile")
    else:
        print("some projects fail")
