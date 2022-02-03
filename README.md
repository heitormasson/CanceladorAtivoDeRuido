# CanceladorAtivoDeRuido
 Software para implementação de um controlador ativo de ruído sonoro na plataforma C2000 Delfino MCU F28379D LaunchPad, desenvolvido como parte do Trabalho de Conclusão do Curso de Engenharia Elétrica, da Escola de Engenharia de São Carlos, por Heitor Amantini Masson.

 
 Esse projeto deve ser utilizado pela IDE Code Composer Studio para programação da plataforma C2000 Delfino MCU F28379D LaunchPad. Possui 3 programas principais:
 
 * IdentificadorDoCaminhoDireto.c
  
  Script que implementa um filtro adaptativo utilizado para a caracterização do caminho direto do sistema
  
 * AgloritmoPrincipal.c
  
  Script usado para implementação de um sistema de cancelamento ativo Feedforward de Banda Larga através do algoritmo FXLMS

 * AlgoritmoAuxiliar.c
  
  Versão simplificada do algoritmo principal menos sucetível às imprecisões dos sensores acústicos.
