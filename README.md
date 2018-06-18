Luso Core v1.0.2.3
===============================

https://www.lusoco.in/

Copyright (c) 2009-2015 Bitcoin Core Developers

Copyright (c) 2011-2015 Litecoin Core Developers

Copyright (c) 2014-2017 Dash Core Developers

Copyright (c) 2017-2018 LusoCoin Developers


License
-------

Luso Core foi lançado usando os termos de licença MIT. Ver [COPYING](COPYING) para mais informação ou ver https://opensource.org/licenses/MIT.


O que é a LUSO?
-------
LusoCoin, é um fork do trabalho feito pela equipa da moeda DASH, anteriormente conhecida como DarkCoin, uma tecnologia que tem o melhor interesse nos seus consumidores e investidores, com funcionalidades de performance, privacidade, segurança e capitalização inexistentes em moedas como o Bitcoin.

Em tempos em que os bancos e os donos do dinheiro visam a globalização para fins de controlo, manipulação e execução de várias agendas ,o Nacionalismo é a nova forma de resistência.


Masternode Share
Max cap: 60%
Max cap LUSO IP: 75%


static const char * Nations[] = { "PT", "BR", "AO", "MZ", "GW", "TL", "CV", "GQ", "MO", "ST" };
10 Countries list: Portugal, Brazil, Angola, Mozambique, Guinea-Bissau, East Timor, Equatorial Guinea, Macau, Cape Verde, São Tomé and Príncipe


Especificações técnicas
-------
Ticker: LUSO

Algo: X11

Reward: 30 LUSO/block

Block space: 90sec (1:30m)

Portas:

  Masternode:

    MAINNET 25040

    TESTNET 12504

    REGTEST 22504

  RPCPort:

    MAINNET 25041

    TESTNET 12505

    REGTEST 22505


Testnet:
  ./src/lusod -testnet


Processo de desenvolvimento
-------
  Os desenvolvedores trabalham em suas próprias árvores e, em seguida, enviam pedidos de "push" quando acham que sua funcionalidade ou correção está pronta.

  Se é uma mudança simples / trivial / não polêmica, então um dos membros da equipe de desenvolvimento da moeda Luso simplesmente a puxa.

  O patch será aceite quando houver um amplo consenso de que é uma coisa boa. Os desenvolvedores devem esperar retrabalhar e reenviar patches se o código não corresponder às convenções de codificação do projeto (consulte doc / coding.md) ou forem controversos.

  O ramo mestre é regularmente construído e testado, mas não é garantido que seja completamente estável. As tags são criadas regularmente para indicar novas versões estaveis da moeda Luso.

  Compilando para depuração

  Execute configure com a opção --enable-debug, depois faça. Ou execute configure com CXXFLAGS = "- g -ggdb -O0" ou qualquer outra sinalização de depuração que você precise.

  debug.log

  Se o código estiver se comportando estranhamente, dê uma olhada no arquivo debug.log no diretório de dados; mensagens de erro e depuração são escritas lá.

  A opção de linha de comando -debug = ... controla a depuração; correr com apenas -debug ativará todas as categorias (e lhe dará um grande arquivo debug.log).

  O código Qt roteia a saída qDebug () para debug.log na categoria "qt": execute com -debug = qt para vê-lo.


Compilar a LUSO
-------

  --- Dependencias
    sudo apt-get install build-essential libtool automake autotools-dev autoconf pkg-config libssl-dev libgmp3-dev libevent-dev bsdmainutils libprotobuf-dev protobuf-compiler qttools5-dev-tools libqrencode-dev libboost-all-dev libboost-dev libminiupnpc-dev

    --- Boost
    sudo apt-get update
    wget -c 'http://sourceforge.net/projects/boost/files/boost/1.48.0/boost_1_48_0.tar.bz2/download'
    tar xf download
    cd boost_1_48_0
    ./bootstrap.sh
    sudo ./b2 install

    wget -c 'http://sourceforge.net/projects/boost/files/boost/1.62.0/boost_1_62_0.tar.bz2/download'
    tar xf download
    cd boost_1_62_0
    ./bootstrap.sh
    sudo ./b2 install

    --- Install Berkley DB v4.8
    sudo add-apt-repository ppa:bitcoin/bitcoin
    sudo apt-get update
    sudo apt-get install libdb4.8-dev libdb4.8++-dev

 - OR

    wget http://download.oracle.com/berkeley-db/db-4.8.30.zip
    unzip db-4.8.30.zip
    cd db-4.8.30
    cd build_unix/
    ../dist/configure --prefix=/usr/local --enable-cxx
    make
    make install

cd pastadodaemon
./configure
make

Se obtiveres erros, tenta incluir directamente as dependencias ou fazer disable de funcionalidades:
./configure help
ex:
  ./configure CPPFLAGS="-I/usr/local/BerkeleyDB.4.8/include -O2" LDFLAGS="-L/usr/local/BerkeleyDB.4.8/lib" --disable-tests
  ./configure CPPFLAGS="-I/usr/include -O2" LDFLAGS="-L/usr/lib" --disable-tests --with-gui=qt4

ou

cd depends; make && cd ..; ./configure --prefix=`pwd`/depends/x86_64-pc-linux-gnu --disable-tests && make
