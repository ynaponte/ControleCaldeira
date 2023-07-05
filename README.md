# ControleCaldeira
Repositório com os códigos utilizados no trabalho final da disciplina de Software em Tempo Real, 2023.1

A aplicação em java é o simulador de caldeira, que nos foi fornecido no trabalho.
Os códigos em C fazem todos partes do controlador, sendo responsáveis por monitores, threads, armazenamento
comunicação entre simulador e controlador, entre outras funções.
O código em C deve ser roda em um sistema linux, pois utiliza bibliotecas Unix.

Para executar o projeto, o simulador deve ser executado via linha de comando, adicionando
ao final a porta UDP desejada para comunicação entre simulador e controlador (utiliza-se
a 4545).
Para executar o controlador, o mesmo deve também ser executado via linha de comando, no
padrão "./main <ip_address> <UDP>". Se estiver utilizando um sistema Uninx, o ip a ser
utilizado é o último que aparece ao se executar o comando "ip r". Caso se esteja utilizando
o WSL, esse se torna o primeiro IP, logo após "default via".
