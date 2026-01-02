/**
 * font painting test
 * @author Tobias Weber
 * @date 26-dec-2025
 * @license see 'LICENSE.GPL' file
 */

#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QFont>
#include <QFontDatabase>
#include <QFile>

#include <iostream>


#define IMG_WIDTH  1024
#define IMG_HEIGHT 1024
#define FONT_SIZE  32.


static void list_fonts()
{
	std::cout << "Available font families:\n";

	std::size_t idx = 0;
	for(const QString& font : QFontDatabase::families())
	{
		std::cout << "\t(" << idx + 1 << ") " << font.toStdString() << std::endl;
		++idx;
	}
}


static void write_text(QPaintDevice& dev)
{
	QPainter painter{&dev};
	painter.setPen(QPen{QColor{0x00, 0x00, 0x00}});
	//painter.drawLine(QLineF{QPointF{100., 100.}, QPointF{200., 200.}});

	qreal x = 10.;
	qreal y = FONT_SIZE;
	qreal dx = 165.;

	std::size_t idx = 0;
	for(const QString& family : QFontDatabase::families())
	{
		QFont font{family};
		font.setPointSize(FONT_SIZE);
		font.setHintingPreference(QFont::PreferFullHinting);
		//font.setKerning(false);
		//font.setFixedPitch(true);

		painter.setFont(font);
		painter.drawText(QPointF{x, y}, QString("%1 Test").arg(idx+1));
		y += FONT_SIZE;

		if(y + FONT_SIZE > IMG_HEIGHT)
		{
			x += dx;
			y = FONT_SIZE;
		}

		if(x + dx > IMG_WIDTH || y + FONT_SIZE > IMG_HEIGHT)
			break;
		++idx;
	}
}


static bool save_image(QImage& img, const char* filename)
{
	//QByteArray arr;
	//QBuffer buf{&arr};
	QFile buf{filename};

	return img.save(&buf, "png");
}


int main(int argc, char** argv)
{
	auto app = std::make_unique<QApplication>(argc, argv);
	list_fonts();

	QImage img{IMG_WIDTH, IMG_HEIGHT, QImage::Format_RGB32};
	img.fill(0xffffff);

	write_text(img);

	if(!save_image(img, "fonts.png"))
	{
		std::cerr << "Error: Could not write image to buffer." << std::endl;
		return -1;
	}

	return 0;
}
