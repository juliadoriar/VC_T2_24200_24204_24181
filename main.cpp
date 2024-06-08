#include <iostream>
#include <string>
#include <chrono>
#include <opencv2\opencv.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\videoio.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <fstream>
#include <vector>
#include <filesystem>

extern "C"
{
#include "vc.h"
}

void vc_timer(void)
{
	static bool running = false;
	static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

	if (!running)
	{
		running = true;
	}
	else
	{
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

		// Tempo em segundos.
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
		double nseconds = time_span.count();

		std::cout << "Tempo decorrido: " << nseconds << "segundos" << std::endl;
		std::cout << "Pressione qualquer tecla para continuar...\n";
		std::cin.get();
	}
}

// // Tabela de mapeamento de cores HSV para valores de resistores
// std::map<std::string, cv::Scalar> colorMap = {
//     {"Preto", cv::Scalar(0, 0, 0)},
//     {"Castanho", cv::Scalar(20, 255, 150)},
//     {"Vermelho", cv::Scalar(0, 255, 255)},
//     {"Laranja", cv::Scalar(10, 255, 255)},
//     {"Amarelo", cv::Scalar(30, 255, 255)},
//     {"Verde", cv::Scalar(60, 255, 255)},
//     {"Azul", cv::Scalar(120, 255, 255)},
//     {"Violeta", cv::Scalar(150, 255, 255)},
//     {"Cinza", cv::Scalar(0, 0, 100)},
//     {"Branco", cv::Scalar(0, 0, 255)}
// };

// Tabela de cores
std::map<std::string, int> colorValueMap = {
    {"Preto", 0},
    {"Castanho", 1},
    {"Vermelho", 2},
    {"Laranja", 3},
    {"Amarelo", 4},
    {"Verde", 5},
    {"Azul", 6},
    {"Violeta", 7},
    {"Cinzento", 8},
    {"Branco", 9}
};

std::map<std::string, int> multiplierMap = {
    {"Preto", 1},
    {"Castanho", 10},
    {"Vermelho", 100},
    {"Laranja", 1000},
    {"Amarelo", 10000},
    {"Verde", 100000},
    {"Azul", 1000000},
    {"Violeta", 10000000},
    {"Cinzento", 100000000},
    {"Branco", 1000000000}
};

std::map<std::string, int> colorHueMap = {
    {"Preto", 0},
    {"Castanho", 20},
    {"Vermelho", 0},
    {"Laranja", 30},
    {"Amarelo", 60},
    {"Verde", 120},
    {"Azul", 240},
    {"Violeta", 270},
    {"Cinzento", 0},
    {"Branco", 0}
};

std::string identificarCor(int hue) {
    for (const auto& color : colorHueMap) {
        if (abs(hue - color.second) < 10) {
            return color.first;
        }
    }
    return "Desconhecido";
}

// Função para identificar a cor dominante em uma região da imagem
/*std::string identifyDominantColor(cv::Mat& hsvImage) {
	
	// Calcular o histograma do canal H (Hue)
    std::vector<cv::Mat> hsvChannels;
    split(hsvImage, hsvChannels);
    int histSize = 180;
    float range[] = {0, 180};
    const float* histRange = {range};
    cv::Mat hist;
    calcHist(&hsvChannels[0], 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

    // Encontrar o bin com o maior valor no histograma
    cv::Point maxLoc;
    minMaxLoc(hist, 0, 0, 0, &maxLoc);

    // Retornar a cor dominante baseada no valor de Hue
    for (auto const& color : colorMap) {
        if (abs(maxLoc.y - color.second[0]) < 10) {
            return color.first;
        }
    }

    return "Desconhecido";
}*/

int main(void)
{
	// Vídeo
	// char videofile[120] = "../video_resistors.mp4";
	// char videofile[120] = "C:/Users/julia/OneDrive/JULIA/Documentos/GitHub/VC_T2_24200_24204_24181/video_resistors.mp4";
	char videofile[120] = "C:/VC_T2_24200_24204_24181/video_resistors.mp4";
	cv::VideoCapture capture; // Objeto para captura de vídeo
	struct
	{
		int width, height;
		int ntotalframes;
		int fps;
		int nframe;
	} video;
	// Outros
	std::string str;
	int key = 0;

	/* Leitura de vídeo de um ficheiro */
	/* NOTA IMPORTANTE:
	O ficheiro video.avi deverá estar localizado no mesmo directório que o ficheiro de código fonte.
	*/
	capture.open(videofile);

	/* Em alternativa, abrir captura de vídeo pela Webcam #0 */
	// capture.open(0, cv::CAP_DSHOW); // Pode-se utilizar apenas capture.open(0);

	/* Verifica se foi possível abrir o ficheiro de vídeo */
	if (!capture.isOpened())
	{
		std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
		return 1;
	}

	/* Número total de frames no vídeo */
	video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
	/* Frame rate do vídeo */
	video.fps = (int)capture.get(cv::CAP_PROP_FPS);
	/* Resolução do vídeo */
	video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
	video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

	/* Cria uma janela para exibir o vídeo */
	cv::namedWindow("VC - VIDEO", cv::WINDOW_AUTOSIZE);

	/* Inicia o timer */
	vc_timer();

	cv::Mat frame;
	
	// Cria uma nova imagem IVC
	IVC *image = vc_image_new(video.width, video.height, 3, 255);
	IVC *image2 = vc_image_new(video.width, video.height, 1, 255);
	IVC *imagemDilatada = vc_image_new(video.width, video.height, 1, 255);
	IVC *imagemEtiquetada = vc_image_new(video.width, video.height, 1, 255);
	IVC *imagemHSV = vc_image_new(video.width, video.height, 3, 255);
	IVC *image6 = vc_image_new(video.width, video.height, 3, 255);

	int contadorResistencias = 0;

	while (key != 'q')
	{
		/* Leitura de uma frame do vídeo */
		capture.read(frame);

		/* Verifica se conseguiu ler a frame */
		if (frame.empty())
			break;

		/* Número da frame a processar */
		video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

		/* Exemplo de inserçãoo texto na frame */
		str = std::string("RESOLUCAO: ").append(std::to_string(video.width)).append("x").append(std::to_string(video.height));
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("TOTAL DE FRAMES: ").append(std::to_string(video.ntotalframes));
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 50), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("FRAME RATE: ").append(std::to_string(video.fps));
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 75), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);
		str = std::string("N. DA FRAME: ").append(std::to_string(video.nframe));
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 0), 2);
		cv::putText(frame, str, cv::Point(20, 100), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 1);

		// Faça o seu código aqui...
		// +++++++++++++++++++++++++


		// Copia dados de imagem da estrutura cv::Mat para uma estrutura IVC
		memcpy(image->data, frame.data, video.width * video.height * 3);
		memcpy(image6->data, frame.data, video.width * video.height * 3);

		// Converte imagem BGR para HSV
		vc_rgb_to_hsv(image);

		// Copia imagem HSV
		memcpy(imagemHSV->data, image->data, video.width * video.height * 3);

		// Segmentação da imagem HSV
		//vc_hsv_segmentation(image, 0, 200, 40, 60, 40, 75);
		vc_hsv_segmentation(image, 0, 200, 40, 60, 40, 75);


		// Tranforma imagem HSV em imagem binária
		vc_3chanels_to_1(image, image2);
		//vc_3chanels_to_1_binary(image, image2);

		//vc_gray_histogram_show(image2, imagemDilatada);

		//vc_gray_dilate(image2, imagemDilatada, 15);
		//vc_binary_dilate(image2, imagemDilatada, 30);

		// Faz a dilatação da imagem binária
		// Converte IVC para cv::Mat
        cv::Mat matImagemBinaria(video.height, video.width, CV_8UC1, image2->data);
        cv::Mat matDilatada;

        // Cria kernel
        int kernelSize = 47;
		//int kernelSize2 = 3;
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize, kernelSize));
		//cv::Mat kernelErosion = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(kernelSize2, kernelSize2));

		// Aplica a função cv::erode
		//cv::erode(matImagemBinaria, matErodida, kernelErosion);

        // Aplica a função cv::dilate
        //cv::dilate(matErodida, matDilatada, kernel);
        cv::dilate(matImagemBinaria, matDilatada, kernel);

        // Converte de volta para IVC
        memcpy(imagemDilatada->data, matDilatada.data, video.width * video.height);


		// // Encontra os contornos na imagem binarizada
		// std::vector<std::vector<cv::Point>> contours;
		// cv::findContours(gray_frame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// // Desenha os contornos (caixas delimitadoras) na imagem original
		// for (size_t i = 0; i < contours.size(); i++) {
		// 	cv::Rect boundingBox = cv::boundingRect(contours[i]);
		// 	cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
		// }

		// Etiquetagem dos blobs
		int nlabels;
		OVC *blobs = vc_binary_blob_labelling(imagemDilatada, imagemEtiquetada, &nlabels);

		// Extração de informação dos blobs
		vc_binary_blob_info(imagemEtiquetada, blobs, nlabels);

		// Converte IVC para cv::Mat
		cv::Mat hsvImage(image->height, image->width, CV_8UC3, imagemHSV->data);
		
		
		if (blobs != nullptr)
		{
		// Definir as regiões das faixas de acordo com a posição das resistências contidas nas bounding boxes
		cv::Rect band1(blobs->xc, blobs->yc, 5, blobs->height);
		cv::Rect band2(blobs->xc + 5, blobs->yc, 5, blobs->height);
		cv::Rect band3(blobs->xc + 5, blobs->yc, 5, blobs->height);
		cv::Rect band4(blobs->xc + 5, blobs->yc, 5, blobs->height);
		//cv::Rect band5(blobs->x + 60, blobs->y, 15, 5);

        //cv::Rect band1(30, 50, 10, 100);
		
		
		int altura = video.width / 2;
		const float tolerance = 3;

		// Verifica se o blob é uma resistência
		if (blobs->area > 15000 && blobs->area < 28000 &&blobs->perimeter > 500 && blobs->perimeter < 700 && blobs->height < 130 && blobs->height > 85)
		{
			// Desenha as bounding boxes
			vc_draw_boundingbox(imagemHSV, blobs);
			vc_draw_center_of_mass(imagemHSV, blobs, nlabels, 10, 255);

			

			// Quando o centro de massa passar pelo centro da tela, contar um blob como resistência
			if (abs(blobs->yc - altura) <= tolerance)
			{
				std::cout << "Resistencia detectada" << std::endl;
				contadorResistencias++;

				// Identificar todas as cores na linha do centro de massa dentro do blob
				cv::Mat linha = hsvImage.row(blobs->yc);

				// Delimitar a área do blob na linha
				int startX = std::max(0, blobs->xc - blobs->width / 2);
				int endX = std::min(linha.cols - 1, blobs->xc + blobs->width / 2);
				cv::Mat linhaBlob = linha(cv::Range::all(), cv::Range(startX, endX));

				std::vector<cv::Mat> hsvChannels;
				cv::split(linhaBlob, hsvChannels);

				int segmentSize = linhaBlob.cols / 4; // Tamanho do segmento
				std::vector<std::string> coresResistor;

				for (int i = 0; i < linhaBlob.cols; i += segmentSize) 
				{
					cv::Rect roi(i, 0, std::min(segmentSize, linhaBlob.cols - i), 1);
					cv::Mat segment = linhaBlob(roi);

					std::vector<cv::Mat> segmentChannels;
					cv::split(segment, segmentChannels);

					int histSize = 180;
					float range[] = {0, 180};
					const float* histRange = {range};
					cv::Mat hist;
					cv::calcHist(&segmentChannels[0], 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

					// Encontrar o bin com o maior valor no histograma
					cv::Point maxLoc;
					cv::minMaxLoc(hist, 0, 0, 0, &maxLoc);

					// Retornar a cor dominante baseada no valor de Hue
					std::string corDominante = identificarCor(maxLoc.y);
					coresResistor.push_back(corDominante);
					// for (const auto& color : colorValueMap) {
					// 	if (abs(maxLoc.y - color.second) < 10) {
					// 		coresResistor.push_back(color.first);
					// 		break;
					// 	}
					// }
				}

				// Exibir as cores na ordem
				for (const auto& cor : coresResistor) {
					std::cout << "Cor: " << cor << std::endl;
				}

				// Verificar se 4 cores foram detectadas
				if (coresResistor.size() == 4) {
					// Calcular o valor da resistência
					int digit1 = colorValueMap[coresResistor[0]];
					int digit2 = colorValueMap[coresResistor[1]];
					int digit3 = colorValueMap[coresResistor[2]];
					int multiplier = multiplierMap[coresResistor[3]];

					int resistencia = (digit1 * 10 + digit2) * multiplier;
					std::cout << "Valor da resistência: " << resistencia << " ohms" << std::endl;
				} else {
					std::cout << "Erro: Não foram detectadas 4 cores." << std::endl;
				}

				// // Identificar a cor dominante de cada faixa
				// std::string color1 = identifyDominantColor(hsvImage(band1));
				// std::cout << "Faixa 1: " << color1 << std::endl;
				// std::string color2 = identifyDominantColor(hsvImage(band2));
				// std::cout << "Faixa 2: " << color2 << std::endl;
				// std::string color3 = identifyDominantColor(hsvImage(band3));
				// std::cout << "Faixa 3: " << color3 << std::endl;
				// std::string color4 = identifyDominantColor(hsvImage(band4));
				// std::cout << "Faixa 4: " << color4 << std::endl;
			}
			

		}
		}

		// Extrair as cores das resistencias para calcular o seu valor em ohm
		
		//Tabela de resistências
		//Cores: Preto, Castanho, Vermelho, Laranja, Amarelo, Verde, Azul, Violeta, Cinzento, Branco
		//Valores: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
		//Tolerância: Ouro, Prata
		//Multiplicador: Preto, Castanho, Vermelho, Laranja, Amarelo, Verde, Azul, Violeta, Cinzento, Branco
		//Valores: 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000



			//Escrever a area e o perimetro de cada blob
			// for (int i = 0; i < nlabels; i++)
			// {
			// 	std::cout << "Frame " << video.nframe << "\n";
			// 	std::cout << "Blob " << i << " Area: " << blobs[i].area << " Perimetro: " << blobs[i].perimeter << std::endl;
			// 	std::cout << "Blob " << i << " X: " << blobs[i].x << " Y: " << blobs[i].y << std::endl;
			// 	std::cout << "Blob " << i << " Width: " << blobs[i].width << " Height: " << blobs[i].height << std::endl;
			// 	std::cout << "Blob " << i << " Xc: " << blobs[i].xc << " Yc: " << blobs[i].yc << std::endl;
			// 	std::cout << "Blob " << i << " Label: " << blobs[i].label << std::endl;
			// }
		

		//  Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		//cv::Mat binary_frame(video.height, video.width, CV_8UC1);
		//memcpy(binary_frame.data, imagemDilatada->data, video.width * video.height);
		memcpy(frame.data, image6->data, video.width * video.height * 3);

		// +++++++++++++++++++++++++

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);
		//cv::imshow("VC - VIDEO", binary_frame);

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);

		// Interrompe a execução após o frame 780
		if (video.nframe == 780)
		{
			break;
		}

	}

	std::cout << "Numero de resistencias: " << contadorResistencias << std::endl;

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	// Liberta a memória da imagem IVC que havia sido criada
	vc_image_free(image);
	vc_image_free(image2);
	vc_image_free(imagemDilatada);
	vc_image_free(imagemEtiquetada);
	vc_image_free(imagemHSV);
	vc_image_free(image6);

	return 0;
}
