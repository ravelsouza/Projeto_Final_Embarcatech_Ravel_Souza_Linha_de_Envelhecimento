# Sistema Embarcado para Testes Automatizados de Luminárias e Displays em Linhas de Envelhecimento (Aging Line)

## 📑 Descrição

Este projeto propõe a automação de uma Linha de Envelhecimento (Aging Line) utilizada na indústria para testar a durabilidade e qualidade de luminárias e displays. Através de um sistema embarcado com a placa **Raspberry Pi Pico W**, foi desenvolvido um simulador que monitora o funcionamento dos dispositivos em teste, identificando falhas, medindo variáveis elétricas e otimizando o processo de controle de qualidade.

## 🚀 Objetivos

- Aplicar conhecimentos em sistemas embarcados.
- Contribuir com a comunidade open-source.
- Otimizar processos de controle de qualidade na indústria.
- Reduzir tempo de testes, mão de obra e consumo energético.
- Aumentar a eficiência e segurança dos processos industriais.
- Prevenir doenças ocupacionais associadas ao monitoramento manual da Aging Line.

## 🔧 Tecnologias e Ferramentas

- **Hardware:**  
  - Raspberry Pi Pico W  
  - Matriz de LEDs WS2812  
  - Resistores de 33 Ohms  
  - Pushbuttons  
  - Buzzers  
  - Display OLED SSD1306  
  - Protoboard e componentes eletrônicos  

- **Software:**  
  - Linguagem C (SDK Raspberry Pi Pico)  
  - Python + Flask (Servidor de Dashboard)  
  - Simuladores: Wokwi, EasyEDA  
  - Editor: VS Code  

## ⚙️ Arquitetura do Sistema

- **Hardware:**  
  - **Controlador:** Gerencia todo o sistema embarcado.  
  - **Acionador:** Dois botões para iniciar e finalizar o teste.  
  - **Simulador:** LEDs que representam luminárias na Aging Line.  
  - **Verificador:** Mede tensão, corrente e estado dos LEDs.  
  - **Expositor:** Matriz de LEDs e servidor Flask para dashboard.  

- **Software:**  
  - Firmware desenvolvido em C para controle do hardware.  
  - Servidor Flask em Python que recebe dados via UART e exibe no dashboard web.

## 🖥️ Dashboard

- LEDs **verdes** representam luminárias funcionando.  
- LEDs **vermelhos** indicam luminárias com falha.  
- Informações exibidas em tempo real:  
  - Estado dos LEDs (aceso/apagado)  
  - Quantidade de LEDs funcionando e com falha  
  - Tensão média  
  - Corrente média e total  
  - Tempo de teste decorrido  

## 📦 Organização do Projeto

