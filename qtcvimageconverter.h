#ifndef QTCVIMAGECONVERTER_H
#define QTCVIMAGECONVERTER_H

#include <opencv2/opencv.hpp>
#include <QImage>

namespace QtCVImageConverter
{

inline cv::Mat QImage2CvMat(const QImage & input_image)
{
    switch (input_image.format())
    {
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        return cv::Mat(input_image.height(),
                       input_image.width(),
                       CV_8UC4,
                       const_cast<uchar*>(input_image.bits()),
                       static_cast<size_t>(input_image.bytesPerLine()));
    case QImage::Format_Indexed8:
        return cv::Mat(input_image.height(),
                       input_image.width(),
                       CV_8UC4,
                       const_cast<uchar*>(input_image.bits()),
                       static_cast<size_t>(input_image.bytesPerLine()));
    case QImage::Format_RGB32:
    case QImage::Format_RGB888:
    {
        QImage temp = input_image;
        if (input_image.format() == QImage::Format_RGB888)
            temp.convertToFormat(QImage::Format_RGB32);
        temp = temp.rgbSwapped();
        return cv::Mat(temp.height(),
                       temp.width(),
                       CV_8UC3,
                       const_cast<uchar*>(temp.bits()),
                       static_cast<size_t>(temp.bytesPerLine()));
    }
    default:
        qWarning() << "QtCVImageConverter() -- Unsupported QImage Input Format:" << input_image.format();
    }
    return cv::Mat();
}

inline cv::Mat QPixmap2CvMat(const QPixmap & input_pixmap)
{
    return QImage2CvMat(input_pixmap.toImage());
}

inline QImage CvMat2QImage(const cv::Mat & input_mat)
{
    switch (input_mat.type())
    {
    case CV_8UC4:
        return QImage(input_mat.data,
                      input_mat.cols,
                      input_mat.rows,
                      static_cast<int>(input_mat.step),
                      QImage::Format_ARGB32);
    case CV_8UC3:
        return QImage(input_mat.data,
                      input_mat.cols,
                      input_mat.rows,
                      static_cast<int>(input_mat.step),
                      QImage::Format_RGB888).rgbSwapped();
    case CV_8UC1:
#if QT_VERSION >= QT_VERSION_CHECK(5,5,0)
        return QImage(input_mat.data,
                      input_mat.cols,
                      input_mat.rows,
                      static_cast<int>(input_mat.step),
                      QImage::Format_Grayscale8);
#else
        QVector<QRgb> colorTable;
        if (colorTable.isEmpty())
        {
            colorTable.resize(256);
            for (int i = 0; i < 256; ++i)
                colorTable[i] = qRgb(i,i,i);
        }
        QImage temp(input_mat.data,
                    input_mat.cols,
                    input_mat.rows,
                    static_cast<int>(input_mat.step),
                    QImage::Format_Indexed8);
        temp.setColorTable(colorTable);
        return temp;
#endif
    default:
        qWarning() << "QtCVImageConverter() -- Unsupported CvMat Input Type:" << input_mat.type();
    }
    return QImage();
}

inline QPixmap CvMat2QPixmap(const cv::Mat & input_mat)
{
    return QPixmap::fromImage(CvMat2QImage(input_mat));
}

}

#endif // QTCVIMAGECONVERTER_H
