import os
import sys
import re
import json

mode = str(sys.argv[1])
pathMSG = "data/msg.txt"
file = open(pathMSG,"w")

# Argumentos 1:Modo 2:Origen 3:Destino 1.4:TextoABuscar 2.4:Tiempo 5:Origen(No el ./) 6:isFound 7:Resultados (lista)

match mode:
    case "1": # Mesaje de busqueda
        msg = {
            'origen' : sys.argv[2],
            'destino' : sys.argv[3],
            'contexto' : {
                'txtToSearch' : sys.argv[4]
            },
        }
        file.write(json.dumps(msg))
        print(msg)
    case "2":
        msg = { # Mensaje de respuesta
            'origen' : sys.argv[2],
            'destino' : sys.argv[3],
            'contexto' : {
                'tiempo' : sys.argv[4],
                'ori' : sys.argv[5],
                'isFound' : sys.argv[6],
                'resultados' : json.loads(sys.argv[7]), # Debe ser una lista
            }
        }
        file.write(json.dumps(msg))
        print(msg)