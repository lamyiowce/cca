#define SFML_STATIC
#include<cstdio>
#include<cstring>
#include<unistd.h>
#include<vector>
#include<SFML/Window.hpp>
#include<SFML/Graphics.hpp>
#include<omp.h>
using namespace std;
using namespace sf;
#include "colors.hpp"
const int MAXWIDTH = 1004, MAXHEIGHT = 1004;

int seed, widthInput, heightInput, numberOfStatesInput, speed = 1;
bool continuousInput = 1, cyclicInput = 1;
int frameLimit[] = {30, 10000}; 
Clock FPSClock;

void clear() {
    printf("\x1B[2J\x1B[H");
}

Color getColor (long long state){
	if (state == 0) return Color(10, 10, 10);
	Color ret = Color((state*255)/numberOfStatesInput, 127+(state*128)/numberOfStatesInput, 0);
	return ret;
}

float getFPS(const sf::Time& time) {
     return (1000000.0f / time.asMicroseconds());
}

class Grid : public Drawable, Transformable {
	
private:
	int width, height;
	int numberOfStates;
	bool cyclic;
	bool continuous;
	int tab[MAXWIDTH][MAXHEIGHT][2];
	VertexArray rects;
	int nums[20];
public:	int czas;
	int cellSize;
private:	
	bool eats (int predator, int prey) {
		prey++;
		if (cyclic && prey > numberOfStates) prey = 1;
		return predator == prey;
	}
	void updateCellColor(int x, int y) {
		int p = czas%2;
		rects[4*(x+(y-1)*width)].color = getColor(tab[x][y][p]);
		rects[4*(x+(y-1)*width)+1].color = getColor(tab[x][y][p]);
		rects[4*(x+(y-1)*width)+2].color = getColor(tab[x][y][p]);
		rects[4*(x+(y-1)*width)+3].color = getColor(tab[x][y][p]);
	}
	
	bool updateCell(int x, int y) {
		int p = czas%2, q = (czas+1)%2;
		int newState = tab[x][y][p];
	//	nums[tab[x][y][p]]--;
		for (int i = -1; i <= 1 && newState == tab[x][y][p]; i++)
			for (int j = -1; j <= 1 && newState == tab[x][y][p]; j++)
				if (abs(j^i) == 1 && eats(tab[x+j][y+i][p], tab[x][y][p]))
					newState = tab[x+j][y+i][p];
		tab[x][y][q] = newState;
	//	nums[newState]++;
		updateCellColor(x, y);
		return tab[x][y][p] != tab[x][y][q];
	}
	
	bool updateCellCont(int x, int y) {
		int p = czas%2, q = (czas+1)%2;
		int nb = rand()%4;
		int a = x, b = y;
		if (nb % 2 == 0) { a--; a+= (nb/2)*2; }
		else {b--; b += (nb/2)*2; }
		a%=width;
		b%=height;
		if (a == 0) a = width;
		if (b == 0) b = height;
		
		int akcja = rand()%3; // 0 - zamiana, 1 - zjedzenie, 2 - ekspansja
		//printf("akcja: %d, xy : %d %d, ab: %d %d\n", akcja, x, y, a, b);
		if (akcja == 0){
			tab[x][y][q] = tab[a][b][p];
			tab[a][b][q] = tab[x][y][p];
		}
		else if (akcja == 1){
			if (eats(tab[x][y][p], tab[a][b][p])){
				tab[a][b][q] = 0;
			}
			else if (eats(tab[a][b][p], tab[x][y][p])){
				tab[x][y][q] = 0;
			}
		}
		else if (akcja == 2 && tab[a][b][p] + tab[x][y][p] > 0 && tab[a][b][p]*tab[x][y][p] == 0){
			tab[a][b][q] = max(tab[a][b][p], tab[x][y][p]);
			tab[x][y][q] = max(tab[a][b][p], tab[x][y][p]);
			
		}
		nums[tab[x][y][p]]--;
		nums[tab[a][b][p]]--;
		nums[tab[x][y][q]]++;
		nums[tab[a][b][q]]++;
		tab[x][y][p] = tab[x][y][q];
		tab[a][b][p] = tab[a][b][q];
		updateCellColor(x, y);
		updateCellColor(a, b);
		return tab[x][y][p] != tab[x][y][q];
	}
	
	virtual void draw(RenderTarget& target, RenderStates states) const{
		target.draw(rects, states);
	}
	
public: 
	Grid (int w, int h, int statesNum, bool cyc, bool cont) {
		width = w;
		height = h;
		cellSize = 2;
		numberOfStates = statesNum;
		czas = 0;
		cyclic = cyc;
		continuous = cont;
		
		for (int i = 0; i < 20; i ++)
			nums[i] = 0;
		
		Vector2f rectSize(cellSize, cellSize);
		rects.setPrimitiveType(Quads);
		rects.resize(width*height*4+10);
		
		for (int i = 1; i <= height; i++){
			for (int j = 1; j <= width; j++){
				if (!continuous) tab[j][i][0] = (rand()%numberOfStates) + 1; 
				else tab[j][i][0] = rand()%(numberOfStates+1);
				nums[tab[j][i][0]]++;
				if (continuous) tab[j][i][1] = tab[j][i][0];
				rects[4*(j+(i-1)*width)].position = Vector2f((j-1)*cellSize, (i-1)*cellSize);
				rects[4*(j+(i-1)*width)+1].position = Vector2f(j*cellSize, (i-1)*cellSize);
				rects[4*(j+(i-1)*width)+2].position = Vector2f(j*cellSize, i*cellSize);
				rects[4*(j+(i-1)*width)+3].position = Vector2f((j-1)*cellSize, i*cellSize);
				rects[4*(j+(i-1)*width)].color = getColor(tab[j][i][0]);
				rects[4*(j+(i-1)*width)+1].color = getColor(tab[j][i][0]);
				rects[4*(j+(i-1)*width)+2].color = getColor(tab[j][i][0]);
				rects[4*(j+(i-1)*width)+3].color = getColor(tab[j][i][0]);
			}
		}
	}
	void printInfo(){
		clear();
		printf("\n");
		printf("Uplynelo jednostek czasu: \e[1m%d\e[0m\nLiczebnosc poszczegolnych populacji:\n", czas);
		int all = 0, biggest = 1;
		for (int i = 1; i <= numberOfStates; i++)
		{
			if (nums[i] > nums[biggest]) biggest = i;
		}
		if (continuous) printf("\e[0m%sPustych: \e[1m%*d %s\n", colorBrightText[0].c_str(), 5, nums[0], biggest == 0?"<--":"");
			
		for (int i = 1; i <= numberOfStates; i++)
		{
			printf("\e[0m%sPopulacja nr %d: \e[1m%*d %s\n", colorBrightText[i].c_str(), i, 5, nums[i], biggest == i?"<--":"");
			all+=nums[i];
		}
		printf("\e[0mWszystkich: \e[1m%d\e[0m\n", all);
		printf("\e[0mseed: %d, liczba populacji: %d, rozmiar: %d x %d, konkurencja: %s, czas: %s.\n", seed, numberOfStatesInput, widthInput, heightInput, cyclicInput?"cykliczna":"uporzadkowana (niecykliczna)", continuousInput?"ciagly":"nieciagly");
		
	}
	bool update() {
		bool change = 0;
		if (continuous)
		{
			int a = rand()%width + 1;
			int b = rand()%height + 1;
			change = 1;
			updateCellCont(a, b);
		}
		else
		{
			unsigned int i, j;
			const int p = max(1, omp_get_max_threads());
			int tmp_nums[20][p];
			for (j = 0; j <= numberOfStates; j++)
			for (i = 0; i < omp_get_max_threads(); i++)
			{
				tmp_nums[j][i] = 0;
			}
			#pragma omp parallel for shared(change) private(i, j)
			for (i = 1; i <= height; i++)
			{
				for (j = 1; j <= width; j++)
				{
					tmp_nums[tab[j][i][czas%2]][omp_get_thread_num()]--;
					change |= updateCell(j, i);
					tmp_nums[tab[j][i][(czas+1)%2]][omp_get_thread_num()]++;
				}
			}
			for (j = 0; j <= numberOfStates; j++)
			for (i = 0; i < omp_get_max_threads(); i++)
			{
				nums[j] += tmp_nums[j][i];
			}
		}
		czas++;
		return change;
	}
};



bool getInput(){
	printf("Podaj ziarno: ");
	scanf ("%d", &seed);
	printf("Podaj liczbe roznych populacji (od 1 do 10): ");
	scanf ("%d", &numberOfStatesInput);
	printf("Podaj rozmiar (szerokosc i wysokosc, nie wieksze niz 1000): ");
	scanf ("%d %d", &widthInput, &heightInput);
	printf("Czy konkurencja ma być cykliczna? (t/n) ");
	char c;
	scanf (" %c", &c);
	cyclicInput = c == 't' || c == 'T';
	printf("Czy czas ma być ciagly? (t/n) ");
	char t;
	scanf (" %c", &t);
	continuousInput = t == 't' || t == 'T';
	if (numberOfStatesInput > 20 || numberOfStatesInput <= 0 || widthInput <= 0 || heightInput <= 0 || (c != 't' && c!= 'n') || (t != 't' && t!= 'n')){
		printf("Podano niepoprawne dane. Koniec programu.\n");
		return false;
	}
	return true;
}

bool setPreset(char* s){
	continuousInput = 0;
	cyclicInput = 1;
	if (!strcmp(s,"big")) {
		numberOfStatesInput = 7;
		widthInput = 200;
		heightInput = 200;
	}
	else if(!strcmp(s, "normal")) {
		numberOfStatesInput = 5;
		widthInput = 100;
		heightInput = 100;
	}
	else if(!strcmp(s, "normalc")) {
		numberOfStatesInput = 5;
		widthInput = 200;
		heightInput = 200;
		continuousInput = 1;
	}
	else if(!strcmp(s, "superbig")) {
		numberOfStatesInput = 7;
		widthInput = 400;
		heightInput = 400;
	}
	else if(!strcmp(s, "db")) {
		numberOfStatesInput = 3;
		widthInput = 200;
		heightInput = 50;
	}
	else if(!strcmp(s, "superbigc")) {
		numberOfStatesInput = 10;
		widthInput = 400;
		heightInput = 400;
		continuousInput = 1;
	}
	else {
		printf("Brak presetu o nazwie %s.\n", s);
		return false;
	}
	printf("Podaj ziarno: ");
	scanf ("%d", &seed);
	return true;
}


int main (int argc, char* argv[])
{
	if (argc  == 1){
		if (!getInput()) return 0;
	} 
	else if (argc == 2){
		if(!setPreset(argv[1])) return 0;
	}
	else {
		printf("Niewlasciwa liczba argumentow.\n");
		return 0;
	}
	srand(seed);
	Grid grid(widthInput, heightInput, numberOfStatesInput, cyclicInput, continuousInput);
	RenderWindow window(VideoMode(widthInput*grid.cellSize, heightInput*grid.cellSize), "CCA - model");
	window.setFramerateLimit(frameLimit[continuousInput]);
	bool stop = 0;
	while(window.isOpen()){
		Event event;
		while(window.pollEvent(event)){
			if (event.type == Event::Closed)
				window.close();
			if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space)
				stop = !stop;
		}
		if (!stop){
			grid.update();
			if(!continuousInput || grid.czas%10000 == 0){
				window.clear(Color::Black);
				window.draw(grid);
				grid.printInfo();
				printf("FPS: %.2lf\n", getFPS(FPSClock.restart()));
				window.display();
			}
		}
	}
	return 0;
}