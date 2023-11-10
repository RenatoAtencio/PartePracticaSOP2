import os
import sys
import re
import json

mode = str(sys.argv[1])
pathMSG = "data/msg.txt"
file = open(pathMSG,"w")

# Argumentos 1:Modo 2:Origen 3:Destino 4:TextoABuscar 5:Tiempo 6:Origen(No el ./) 7:Resultados (lista)

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
        if (sys.argv[7] == "[]"): 
            ori = "false" # Si la lista esta vacia no encontro la palabra
        else: 
            ori = "true"
        msg = { # Mensaje de respuesta
            'origen' : sys.argv[2],
            'destino' : sys.argv[3],
            'contexto' : {
                'txtToSearch' : sys.argv[4],
                'tiempo' : sys.argv[5],
                'ori' : sys.argv[6],
                'isFound' : ori,
                'resultados' : json.loads(sys.argv[7]), # Debe ser una lista
            }
        }
        file.write(json.dumps(msg))