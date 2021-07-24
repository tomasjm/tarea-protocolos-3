Se testeó el programa siguiendo estos pasos:

```git clone https://github.com/tomasjm/tarea-protocolos-entrega```

```cd tarea-protocolos-entrega```

```make```

En una terminal

```sudo ./punto b8:27:eb:15:b1:ca 11:27:bb:44:b1:ca 0 2 3```

En otra terminal

```sudo ./punto 11:27:bb:44:b1:ca b8:27:eb:15:b1:ca 23 22 21```

Para testear el error de las direcciones MAC pueden ser cambiadas libremente.

PINES
NODO  CLOCK  TX  RX     TX2   RX2
1        0    2   3      9     8
2       23   24   25     22    21
3       1     4   5      29    28