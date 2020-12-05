# ComputerGraphics-openGL
## 실제 모델링한 오브젝트의 obj파일을 읽어와 3D 화면으로 렌더링하고 텍스쳐 매핑작업

#### 사용 언어 : C, C++
#### 작업 인원 : 1인 프로젝트(Renderer.cpp의 main, display함수 작성)

## 작업 내용
> ###### 3D Max로 모델링한 오브젝트를 obj 파일로 export해서 visual studio project에서 파일을 읽어와 3D 공간 상에 다시 렌더링(텍스처 매핑 포함)

###### * main() : 렌더링하는데에 필요한 버텍스 정보와 텍스처 이미지 불러오기
	vertex = new Vertex[16453];
	carvn = new Vertex[16613];

	FILE* fp;
	fp = fopen("car.obj", "r");
	int count = 0;
	char ch;
	float x, y, z;
	for (register int j = 0; j < 16453; j++) {
		count = fscanf(fp, "%c %f %f %f", &ch, &x, &y, &z);
		if (count == 4 && ch == 'v') {
			vertex[j].X = x / scale;
			vertex[j].Y = y / scale;
			vertex[j].Z = z / scale;
		}
		else
			j -= 1;
	}
	fclose(fp);

	// carvn.txt 파일 읽어오기
	fp = fopen("carvn.txt", "r");
	char* str;
	for (register int j = 0; j < 16613; j++) {
		count = fscanf(fp, "%s %f %f %f", &str, &x, &y, &z);
		if (count == 4) {
			carvn[j].X = x;
			carvn[j].Y = y;
			carvn[j].Z = z;
		}
	}
	fclose(fp);

	// cart.bmp 파일 읽어오기
	FILE* f = fopen("cart.bmp", "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
											   // extract image height and width from header 
	int width = *(int*)&info[18];
	int height = *(int*)&info[22];
	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size];
	fread(data, sizeof(unsigned char), size, f);
	fclose(f);
	int k = 0;
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++) {
			cart[j][i][0] = data[k * 3 + 2];
			cart[j][i][1] = data[k * 3 + 1];
			cart[j][i][2] = data[k * 3];
			k++;
		}

	// carf.txt 파일 읽어오기
	string map1 = "", map2 = "", map3 = "";
	ifstream fin;
	fin.open("carf.txt");
	for (int i = 0; i < 32922; i++) {
		fin >> ch >> map1 >> map2 >> map3;
		for (int j = 0; j < 12; j++) {
			if (j < 3) {
				carf[i][j] = atoi(map1.substr(0, map1.find("/")).c_str());
				map1.erase(0, map1.find("/") + 1);
			}
			else if (j < 6) {
				carf[i][j] = atoi(map2.substr(0, map2.find("/")).c_str());
				map2.erase(0, map2.find("/") + 1);
			}
			else {
				carf[i][j] = atoi(map3.substr(0, map3.find("/")).c_str());
				map3.erase(0, map3.find("/") + 1);
			}
		}
	}
	fin.close();

	InitializeWindow(argc, argv);

	display();

###### * display() : 실제 렌더링하는 함수
	// 텍스처 매핑
	glTexImage2D(GL_TEXTURE_2D, 0, 3, 255, 255, 0, GL_RGB, GL_UNSIGNED_BYTE, cart);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	...
	
	// 텍스처를 입힐 모델 생성
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_TRIANGLES);
	int vertex_1, vertex_2, vertex_3;
	int normal_1, normal_2, normal_3;
	float newNormal_X, newNormal_Y, newNormal_Z;
	for (int i = 0; i < 32922; i++) {
		vertex_1 = carf[i][0];		// 삼각형을 이루는 점들 중 첫번째 점의 인덱스
		vertex_2 = carf[i][3];		// 삼각형을 이루는 점들 중 두번째 점의 인덱스
		vertex_3 = carf[i][6];		// 삼각형을 이루는 점들 중 세번째 점의 인덱스

		// 위에 인덱스를 가져온 3개의 점들을 가지고 삼각형 생성
		// uv map의 좌표는 임의로 순서대로 (0, 0), (1, 0), (0, 1)로 설정
		glTexCoord2f(0, 0);
		glVertex3f(vertex[vertex_1 - 1].X - 0.1, vertex[vertex_1 - 1].Y, vertex[vertex_1 - 1].Z);			  
		glTexCoord2f(1, 0);				  
		glVertex3f(vertex[vertex_2 - 1].X - 0.1, vertex[vertex_2 - 1].Y, vertex[vertex_2 - 1].Z);
		glTexCoord2f(0, 1);				  
		glVertex3f(vertex[vertex_3 - 1].X - 0.1, vertex[vertex_3 - 1].Y, vertex[vertex_3 - 1].Z);
	}
	glDisable(GL_TEXTURE_2D);
	glEnd();

	//텍스처를 입히지 않는 모델 생성
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < 32922; i++) {
		vertex_1 = carf[i][0];	normal_1 = carf[i][2];	// 첫번째 점의 좌표와 노말벡터
		vertex_2 = carf[i][3];	normal_2 = carf[i][5];	// 두번째 점의 좌표와 노말벡터
		vertex_3 = carf[i][6];	normal_3 = carf[i][8];	// 세번째 점의 좌표와 노말벡터
		
		// 세 점의 노말벡터의 x, y, z 방향의 평균을 각각 계산, 새로운 노말벡터 (newNormal_X, newNormal_Y, newNormal_Z) 생성
		newNormal_X = (carvn[normal_1 - 1].X + carvn[normal_2 - 1].X + carvn[normal_3 - 1].X) / 3;
		newNormal_Y = (carvn[normal_1 - 1].Y + carvn[normal_2 - 1].Y + carvn[normal_3 - 1].Y) / 3;
		newNormal_Z = (carvn[normal_1 - 1].Z + carvn[normal_2 - 1].Z + carvn[normal_3 - 1].Z) / 3;
		
		glNormal3f(newNormal_X, newNormal_Y, newNormal_Z);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(vertex[vertex_1 - 1].X + 0.1, vertex[vertex_1 - 1].Y, vertex[vertex_1 - 1].Z);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(vertex[vertex_2 - 1].X + 0.1, vertex[vertex_2 - 1].Y, vertex[vertex_2 - 1].Z);
		glColor3f(1.0, 1.0, 1.0);
		glVertex3f(vertex[vertex_3 - 1].X + 0.1, vertex[vertex_3 - 1].Y, vertex[vertex_3 - 1].Z);
	}


