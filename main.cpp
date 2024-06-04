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
	IVC *image3 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image4 = vc_image_new(video.width, video.height, 1, 255);
	IVC *image5 = vc_image_new(video.width, video.height, 3, 255);

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
		memcpy(image5->data, frame.data, video.width * video.height * 3);

		vc_rgb_to_hsv(image);

		vc_hsv_segmentation(image, 0, 200, 40, 60, 40, 75);

		//vc_3chanels_to_1(image, image2);
		vc_3chanels_to_1_binary(image, image2);

		//vc_gray_dilate(image2, image3, 15);
		vc_binary_dilate(image2, image3, 30);

		// // Encontra os contornos na imagem binarizada
		// std::vector<std::vector<cv::Point>> contours;
		// cv::findContours(gray_frame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		// // Desenha os contornos (caixas delimitadoras) na imagem original
		// for (size_t i = 0; i < contours.size(); i++) {
		// 	cv::Rect boundingBox = cv::boundingRect(contours[i]);
		// 	cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
		// }

		int nlabels;
		OVC *blobs = vc_binary_blob_labelling(image3, image4, &nlabels);

		vc_binary_blob_info(image4, blobs, nlabels);
		vc_draw_boundingbox(image5, blobs);

		//int nlabels;
		//OVC *blobs = vc_binary_blob_labelling(image3, image4, &nlabels);

		//vc_binary_blob_info(image4, blobs, nlabels);
		//vc_draw_boundingbox(image5, blobs);


		//  Copia dados de imagem da estrutura IVC para uma estrutura cv::Mat
		// cv::Mat binary_frame(video.height, video.width, CV_8UC1);
		// memcpy(binary_frame.data, image4->data, video.width * video.height);
		memcpy(frame.data, image5->data, video.width * video.height);

		// +++++++++++++++++++++++++

		/* Exibe a frame */
		cv::imshow("VC - VIDEO", frame);
		//cv::imshow("VC - VIDEO", binary_frame);

		/* Sai da aplicação, se o utilizador premir a tecla 'q' */
		key = cv::waitKey(1);
	}

	/* Para o timer e exibe o tempo decorrido */
	vc_timer();

	/* Fecha a janela */
	cv::destroyWindow("VC - VIDEO");

	/* Fecha o ficheiro de vídeo */
	capture.release();

	// Liberta a memória da imagem IVC que havia sido criada
	vc_image_free(image);
	vc_image_free(image2);
	vc_image_free(image3);
	vc_image_free(image4);

	return 0;
}
