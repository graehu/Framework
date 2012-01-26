#ifndef TMXLOADER_H
#define TMXLOADER_H


#include <SDL/SDL.h>
#include <SDL/SDL_image.h>


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;


#include "base64.h"
#include "../tinyxml/tinyxml.h"

class TMXLoader : public TiXmlVisitor
{
    public:
        TMXLoader();
        virtual ~TMXLoader();


        void cleanup();


        virtual bool 	VisitEnter  (const TiXmlDocument  &);

        virtual bool 	VisitExit (const TiXmlDocument &);

        virtual bool 	VisitEnter (const TiXmlElement &, const TiXmlAttribute *);

        virtual bool 	VisitExit (const TiXmlElement &);

        virtual bool 	Visit (const TiXmlDeclaration &);

        virtual bool 	Visit (const TiXmlText &);

        virtual bool 	Visit (const TiXmlComment &);

        virtual bool 	Visit (const TiXmlUnknown &);


        bool loadDocument();

        SDL_Surface* getTilesetImage() { return img_tileset; }


        SDL_Surface* getMapImage();

        int getTileWidth() { return m_TileWidth; }
        int getTileHeight() { return m_TileWidth; }
        int getTileSpacing() { return m_TileSpacing; }
        int getTilesetMargin() { return m_TilesetMargin; }
        int getNumMapColumns() { return m_NumMapColumns; }
        int getNumMapRows() { return m_NumMapRows; }

    protected:

    private:
        int m_TileWidth;
        int m_TileHeight;
        int m_TileSpacing;
        int m_TilesetMargin;
        int m_NumMapColumns;
        int m_NumMapRows;

        SDL_Surface * img_tileset,
                    * img_map;
        ofstream myfile;

        vector< vector< int > > m_LayerData;


        void decode_and_store_map_data( string encoded_data );
        SDL_Surface* load_SDL_image( std::string filename );


        void buildMapImage();
        //TiXmlDocument *doc;

};

#endif // TMXLOADER_H
