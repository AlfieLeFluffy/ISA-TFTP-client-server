# ISA-TFTP-client/server

## Adresářová struktura projektu

- Základní zdrojové kódy ```ftfp-server.c``` a ```ftfp-client.c``` jsou v adresáři ```src```
- Všechny vytvořené include soubory jsou v adresáři ```include```
- Vytvořené soubory pro práci s pakety jsou v adresáři ```include/packets```
- Seznam možných ERROR chyb jsou v souboru ```error_code_msg.h``` jsou v adresáři ```include/lists```

## Funkce v ```send_file.c``` a ```recieve_file.c```

Při implemetaci řešení jsem narazil na podivnou to chybu kdy funkce ```recvfrom``` funguje v hlavních funkcích ```main```, ale při jejím zanořní do jakékoliv funkce okmažitě při zavolání uzavřela socket a nevrátila nic kromě chyby errno 880. Pokusy o nalezení řešení byli marné takže algoritmy pro výsílání a pro přijmání souborů jsou přímo v ```main``` funkcích aplikací. Bohužel to vytváří méně přehledný kód, ale snad to nebude příliš překážet.

## Chybějící funkcionalita

- Programy dokázejí pracovat se všemi třemi módy přenosu, ale není nijak implementované mezi nimi přepínat, takže programy automaticky používají ```netascii```.
- Option tsize je implementován jako kontrola na obou stranách přenosu, ale v tvorbě packetu a přijmání paketu nejsou dořešené správné přenosy velikosti.

## Dodatečné informace

- Programy automaticky zruší přenos pokud se uživatel pokusí přepsat již existující soubor v obou typech požadvků.
- Programy se automaticky snaží najít co nejmenší ```blocksize``` a pokud je soubor vetší než 65500 bytů tak používají ```blocksize=65500```.
- Program považuje jako maximum dat co může přenést v ```blocksize``` 65500 pro lepší přehlednost a menší šanci chyby.
- Program používá option ```timeout```, ale automaticky se snaží použít ```timeout=1``` jako výchozí.

## Použítí

Programy používají stejné formáty příkazů jáká jsou v zadání

```tftp-client -h hostname [-p port] [-f filepath] -t dest_filepath```

    -h IP adresa/doménový název vzdáleného serveru
    -p port vzdáleného serveru
        pokud není specifikován předpokládá se výchozí dle specifikace
    -f cesta ke stahovanému souboru na serveru (download)
        pokud není specifikován používá se obsah stdin (upload)
    -t cesta, pod kterou bude soubor na vzdáleném serveru/lokálně uložen

```tftp-server [-p port] root_dirpath```

    -p místní port, na kterém bude server očekávat příchozí spojení
    cesta k adresáři, pod kterým se budou ukládat příchozí soubory

## Testování

Výsledky testování jsou součástí dokumentace.