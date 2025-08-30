# Farm Control MVP

## RFs (Requsitos Funcionais)

- [x] O Usuário deve conseguir alterar a programação das portas logicas
- [x] O usuário deve conseguir adicionar novas portas
- [x] o usuário deve conseguir acionar uma porta independente do horario
- [ ] o usuário deve conseguir acompanhar o cumprimento das execuções durante o dia (histórico)

## RN (Regras de Negócio)
- [ ] O usuário só poderá atualizar a programação se tiver autorização

## RNF (Requisitos não Funcionais)

- [x] Ao ligar a placa, esta deve ativar o modo STA e se conectar a wifi via WPS.
- [x] Caso Já Possua credencial salva, não precisa entrar no modo WPS.
- [x] Quando a internet estiver ok, deve atualizar a Data/Hora, fazendo requisição na API de data/hora 
- [x] Caso não consiga atualizar a data/hora, aguarda 5 segundos e tenta chamada a API novamente. 
- [x] Se a hora estiver OK, pode realizar as tarefas.