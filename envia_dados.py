import serial
import time
import os
from time import localtime, strftime



n_serial = input("Digite o número da serial = ") #seta a serial
n_serial1 = int(n_serial) - 1
ser = serial.Serial("COM"+str(n_serial), 115200, timeout=0.5,parity=serial.PARITY_NONE) # serial Windows



# Entra com quantas medidas vai realizar
#num_medidas = input('Entre com o número de medidas = ')
#num_medidas = 1000000


# Cria o vetor Pacote
Pacote_RX=[0]*52


try:
    while True:
        # Leia uma linha da porta serial
        linha_serial = ser.readline().decode('utf-8').strip()
        # ============= Camada Física - Recebe o pacote
        Pacote_RX = ser.read(52) # faz a leitura de 52 bytes do buffer que rec
        bytes_em_string = Pacote_RX.decode("utf-8") # ("latin1") nao retornou 1 erro se quer
        filename1 = strftime("testando.txt") # arquivo teste
        meu_teste = open(filename1, 'w') #abre o arquivo para escrever
        meu_teste.write(bytes_em_string)
        #meu_teste.write(Pacote_RX)

        if len(Pacote_RX) >= 52:
            print ('>=52')
            print(bytes_em_string)

        else:
            print ('<52')
         
        

        
    ser.close()
    print ('Fim da Execução')  # escreve na tela

except KeyboardInterrupt:
   ser.close()
   meu_teste.close()
   Log_Rota1_B1.close()