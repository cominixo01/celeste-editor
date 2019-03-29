#include "cart.h"

Cart::Cart() {}

void Cart::loadFile(QString cartPath) {
    QFile file(cartPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        throw "Error while reading the cart file";

    std::string cur_del = "header";

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        const std::string lineStd = line.toStdString();
        const char * lineBuffer = lineStd.c_str();

        auto find = std::find(std::begin(delimiters), std::end(delimiters), lineBuffer);

        if (find != std::end(delimiters)) {
            cur_del = lineStd.substr(0, lineStd.size()-1);;
        } else {

            raw_data[cur_del] += lineStd;

        }
    }

    unsigned long gfx_size = raw_data["__gfx__"].size();

    raw_data["gfx_map"] = raw_data["__gfx__"];
    raw_data["gfx_map"].erase(0, gfx_size/2);
    raw_data["__gfx__"].erase(gfx_size/2, gfx_size);

    QImage spritesheet = getSpritesheetImage();
    getAllSprites(spritesheet);

    unsigned long sprite_id = 0;
    int curY = 0;
    int curX = 0;

    // sorry too lazy to make this look nice (it's just removing the new lines)
    raw_data["__map__"].erase(std::remove(raw_data["__map__"].begin(), raw_data["__map__"].end(), '\n'), raw_data["__map__"].end());
    raw_data["gfx_map"].erase(std::remove(raw_data["gfx_map"].begin(), raw_data["gfx_map"].end(), '\n'), raw_data["gfx_map"].end());

    for (unsigned int i = 0; i < raw_data["gfx_map"].length(); i+=2) {
        raw_data["__map__"].append(std::string() + raw_data["gfx_map"][i+1] + raw_data["gfx_map"][i]);
    }

    for (unsigned int i = 0; i < raw_data["__map__"].length(); i+=2) {
        std::string hex_string = std::string() + raw_data["__map__"][i] + raw_data["__map__"][i + 1];

        sprite_id = unsigned(std::strtol(hex_string.c_str(), nullptr, 16));

        map[curY][curX] = int(sprite_id);

        if (curX >= 127) {
            curY += 1;
            curX = 0;
        } else {
            curX += 1;
        }
    }
}

QImage Cart::getMapImage() {

    QImage mapImage = QImage(128*8, 64*8, QImage::Format_RGBA8888);

    mapImage.fill(Qt::black);

    for(int y=0;y<64;y++){
        for(int x=0;x<128;x++){
            QImage sprite = spritesImage[map[y][x]];

            QPoint destPos = QPoint(x*8, y*8);
            QPainter painter(&mapImage);
            painter.drawImage(destPos, sprite);
            painter.end();
        }
    }

    return mapImage;
}

QImage Cart::getLevelImage(int id){
    QImage levelImage = QImage(128, 128, QImage::Format_RGBA8888);

    levelImage.fill(Qt::black);

    int start_x=id%8*16;
    int start_y=int(id/8)*16;

    for(int y=start_y;y<start_y+16;y++){
        for(int x=start_x;x<start_x+16;x++){
            QImage sprite = spritesImage[map[y][x]];

            QPoint destPos = QPoint((x-start_x)*8, (y-start_y)*8);
            QPainter painter(&levelImage);
            painter.drawImage(destPos, sprite);
            painter.end();
        }
    }

    return levelImage;
}

void Cart::getAllSprites(QImage spritesheet) {

    int curX = 0;
    int curY = 0;

    std::vector<QImage> sprites;

    for (unsigned int i = 0; i < raw_data["__gfx__"].size(); i+=8) {

        curX = i % 128;

        curY = int(std::floor(i / 128) * 8);
        QRect rect(curX, curY, 8, 8);
        QImage spriteImage = spritesheet.copy(rect);
        sprites.push_back(spriteImage);
    }

    spritesImage = sprites;
}

QImage Cart::getSpritesheetImage() {

    QImage spritesheet(128, 64, QImage::Format_RGBA8888);

    int color_int = 0;
    int sheetX = 0;
    int sheetY = 0;
    char color;
    for (unsigned int i = 1; i < raw_data["__gfx__"].length(); i++) {
        color = raw_data["__gfx__"][i];
        color_int = int(std::strtol(&color, nullptr, 16));

        if ((sheetX) >= 128) {
            sheetY += 1;
            sheetX = 0;
        } else {
            sheetX += 1;
        }

        if (sheetX < 128) {
            spritesheet.setPixel(sheetX, sheetY, colors[color_int]);
        }


    }
    return spritesheet;
}
