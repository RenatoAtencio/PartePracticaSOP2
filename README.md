# PartePracticaSOP2
Parte practica de la prueba 2 de sistemas operativos

# Compilacion
Usar make en la consola en la carpeta PartePracticaSOP2

# Ejecucion
Para hacer uso del sistema se necesita correr ./searcher , ./memcache y ./invertedindex en 3 consolas diferentes, intentar correr ./searcher , las interacciones entre los programas se pueden ver durante la ejecucion, pero searcher no puede ser usado sin el memcache y si se hace una busqueda que requiera del invertedindex se debe de tener ese programa ejecutandose.
Si sale un problema con token usar make clean y luego make

# Arquitectura
El sistema hace uso de 6 programas
    actualizarJson.cpp : Agrega y elimina informacion del cache.json, toma un msg en un formato (1) y el memorySize, el tamaño del cache.json depende del memorySize
    buscador.cpp : Es el buscador basado en indice invertido, toma path y nombre del archivo .idx, la busqueda del usuario y el topk
    invertedindex.cpp : Programa que actua como servidor para el cliente memcache, toma un msg enviado por el cache y hace la busqueda correspondiente en el indice invertido (Llama al buscador), devuelve un msg en un formato (2)
    memcache.cpp : Programa que actua como cliente del invertedindex y como server del cliente searcher, toma un msg enviado por el searcher, hace la busqueda preliminar en cache.json, si se encuentra manda la respuesta al searcher, sino lo encuentra se conecta y manda un msg al invertedindex para que realize la busqueda.
    searcher.cpp : Es el programa que interactua directamente con el cliente del sistema, toma una busqueda y lo manda al memcache, luego recibe la respuesta correspondiente.
    format.py : Toma datos y crea el mng en el formato correspondiente, tiene 2 modos para los msg de busqueda y respuesta
        modo1: python3 format.py 1 origen destino txtToSearch
        modo2: python3 format.py 2 origen destino txtToSearch tiempo origen(MEMCACHE o INVINDEX) resultados(La lista de resultados de la busqueda)

Se hace uso de 3 archivos env.json guardados en la carpeta config
    invertedindex_env.json: 
    {
        "FROM" : "./searcher",
        "TO" : "./memcache",
        "FILE" : "file.idx",
        "TOPK" : 4
    }
    memcache_env.json:
    {
    "HOST": "./memcache",
    "FRONT": "./searcher",
    "BACK": "./invertedindex",
    "MEMORYSIZE": 4
    }
    searcher_env.json:
    { 
        "FROM": "./searcher",
        "TO": "./memcache" 
    }

Los servers de memcache y invertedindex usan ambos la ip: "127.0.0.1" y los sockets 12345 y 12346 respectivamente.

Se generan 2 tipos de mensajes:
    De busqueda: (Usado de searcher a memcache o memcache a invertedindex)
        {
            "origen" : "origen",
            "destino" : "destino",
            "contexto" : {
                "txtToSearch" : "busqueda"
            }
        }
    De respuesta: (Usado de memcache a searcher o invertedindex a memcache a searcher)
        {
            "origen": "origen",
            "destino": "destino",
            "contexto": {
                "tiempo": "tiempo(ns)",
                "ori": "origen",
                "isFound": "ResultadoBusqueda",
                "txtToSearch":"busqueda",
                "resultados" : [
                    {"archivo": "texto1", "puntaje": "cant1"},
                    {"archivo": "texto2", "puntaje": "cant2"},
                    {"archivo": "texto3", "puntaje": "cant3"},
                    {"archivo": "texto4", "puntaje": "cant4"},
                    {"archivo": "texto5", "puntaje": "cant5"}
                ]
            }
        }