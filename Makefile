game: 
	clang temp2.c -o play -I include -I /opt/homebrew/include/SDL2 -L lib -L /opt/homebrew/lib -l SDL2-2.0.0 -l SDL2_ttf -l SDL2_image
