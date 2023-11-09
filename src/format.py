import os
import sys
import re
import json

mode = str(sys.argv[1])
pathMSG = "data/msg.txt"
file = open(pathMSG,"w")

# Argumentos 1:Modo 2:Origen 3:Destino 1.4:TextoABuscar 2.4:Tiempo 5:Origen(No el ./) 6:Resultados (lista)

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
    case "2":
        if (sys.argv[6] == "[]"): 
            ori = "false" # Si la lista esta vacia no encontro la palabra
        else: 
            ori = "true"
        msg = { # Mensaje de respuesta
            'origen' : sys.argv[2],
            'destino' : sys.argv[3],
            'contexto' : {
                'tiempo' : sys.argv[4],
                'ori' : sys.argv[5],
                'isFound' : ori,
                'resultados' : json.loads(sys.argv[6]), # Debe ser una lista
            }
        }
        file.write(json.dumps(msg))