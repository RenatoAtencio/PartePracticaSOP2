import os
import sys
import re

mode = str(sys.argv[1])
pathMSG = "data/msg.txt"
file = open(pathMSG,"w")

match mode:
    case "1": #Mesaje desde Search a la cache
        msg = {
            'origen' : sys.argv[2],
            'destino' : sys.argv[3],
            'contexto' : {
                'txtToSearch' : sys.argv[4]
            },
        }
        file.write(str(msg))
        print(msg)