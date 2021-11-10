all:mail.cpp 
	g++ -g -o mail mail.cpp `pkg-config --libs --cflags gmime-2.6`
	g++ -g -o recvMail recvMail.cpp Lmime.cpp  `pkg-config --libs --cflags gmime-2.6`
