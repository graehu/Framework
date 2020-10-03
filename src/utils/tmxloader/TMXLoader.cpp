


#include "TMXLoader.h"

TMXLoader::TMXLoader()
{
    //mapWidth = mapHeight = 0;
    img_tileset = NULL;


    // Debug crap
    //ofstream myfile;
    myfile.open ("debug.txt");


}

TMXLoader::~TMXLoader()
{
    //dtor
}

void TMXLoader::cleanup()
{
    //Free the surface
    SDL_FreeSurface( img_tileset );
    SDL_FreeSurface( img_map );


    myfile.close();
}

bool 	TMXLoader::VisitEnter  (const TiXmlDocument  &doc)
{
    return true; // #TODO: for performance, we may not want to return true for each of these callbacks for the visitor pattern.
}

bool 	TMXLoader::VisitExit (const TiXmlDocument &doc)
{
    return true;
}

bool 	TMXLoader::VisitEnter (const TiXmlElement &elem, const TiXmlAttribute *attrib)
{
    if (string("map") == elem.Value()) {
        elem.Attribute("width", &m_NumMapColumns);
        elem.Attribute("height", &m_NumMapRows);

        //elem.Attribute("tileheight");
        //elem.Attribute("tilewidth");

        // #TODO: get width and height, and tilewidth and tileheight
        //m_layer_width =

    }
    else if (string("tileset") == elem.Value()) {

        // Need a new class called Tileset

        elem.Attribute("tilewidth", & m_TileWidth);
        elem.Attribute("tileheight", &m_TileHeight);
        elem.Attribute("spacing", &m_TileSpacing);
        elem.Attribute("margin", &m_TilesetMargin);

        // #TODO: get spacing and margin
    }
    else if (string("image") == elem.Value()) {
        //string attrib = attrib.ValueStr();
        const char* attrib = elem.Attribute("source");

        img_tileset = load_SDL_image(attrib);
    }
    else if (string("layer") == elem.Value()) {
        // We don't neet to use layer width and height yet.
        //elem.Attribute("name");
        //elem.Attribute("width");
        //elem.Attribute("height");
    }
    else if (string("data") == elem.Value()) {
        const char* text = elem.GetText();
        decode_and_store_map_data( text );

        buildMapImage();
    }

    return true;
}

 bool 	TMXLoader::VisitExit (const TiXmlElement &elem)
{
    return true;
}

 bool 	TMXLoader::Visit (const TiXmlDeclaration &dec)
{
    return true;
}

 bool 	TMXLoader::Visit (const TiXmlText &text)
{
    return true;
}

 bool 	TMXLoader::Visit (const TiXmlComment &comment)
{
    return true;
}

 bool 	TMXLoader::Visit (const TiXmlUnknown &unknown)
{
    return true;
}

bool TMXLoader::loadDocument()
{
    TiXmlDocument doc("testa.tmx");

    if ( ! doc.LoadFile() ) {
		return false;
	}

    //TiXmlElement* elem = doc.RootElement();

    doc.Accept(this);

    return true;
}

void TMXLoader::decode_and_store_map_data( string encoded_data )
{
    //const int NUM_LAYER_COLS = 3;
   // const int NUM_LAYER_ROWS = 3;
   // const int NUM_TILES = NUM_LAYER_ROWS * NUM_LAYER_COLS;
    //int m_LayerData[NUM_LAYER_ROWS][NUM_LAYER_COLS];
    //string encoded_data = elem.GetText();
    //string unencoded_data = base64_decode(encoded_data);
    //const char* unencoded_c_str = unencoded_data.c_str();
    //const char* unencoded_c_str = unencoded_data.c_str();
    //m_LayerData.push_back( vector<> );

    vector< int > layerDataRow( getNumMapColumns() );
    int m_LayerRow = 0;
    int m_LayerCol = 0;

    vector<int> unencoded_data = base64_decode(encoded_data);

    for (int i = 0; i < getNumMapRows(); i++)
    {
        m_LayerData.push_back( layerDataRow );
    }

    for (int i = 0; i < unencoded_data.size(); i += 4)
    {
        // Get the grid ID

        int a = unencoded_data[i];
        int b = unencoded_data[i + 1];
        int c = unencoded_data[i + 2];
        int d = unencoded_data[i + 3];

        int gid = a | b << 8 | c << 16 | d << 24;

        m_LayerData[m_LayerRow][m_LayerCol] = gid;

        if ((i + 4) % ( getNumMapColumns() * 4) == 0) {
            m_LayerRow++;
            m_LayerCol = 0;
            //myfile << endl;
        }
        else {
            m_LayerCol++;
        }
    }

//    for (int row = 0; row < NUM_LAYER_ROWS; row++)
//    {
//        for (int col = 0; col < NUM_LAYER_COLS; col++)
//        {
//           myfile << " (" << m_LayerData[row][col] << ") ";
//        }
//
//        myfile << endl;
//    }
}
/*****************************************************/
SDL_Surface* TMXLoader::load_SDL_image( std::string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

void TMXLoader::buildMapImage()
{
    // We must find a way to get these values from the layer at runtime!
    const int NUM_TILESET_COLS = 18;
    const int NUM_TILESET_ROWS = 11;

    //const int MARGIN = 2;
    //const int NUM_TILES = NUM_LAYER_ROWS * NUM_LAYER_COLS;

    img_map = SDL_CreateRGBSurface( SDL_SWSURFACE, getNumMapColumns() * 32, getNumMapRows() * 32, 32, 0, 0, 0, 0 );

    //SDL_Surface *src;
    //SDL_Surface *dst;
    SDL_Rect srcrect;
    SDL_Rect dstrect;

    for (int row = 0; row < getNumMapRows(); row++)
    {
        for (int col = 0; col < getNumMapColumns(); col++)
        {
            int gid = m_LayerData[row][col];

            if (gid == 0)
                continue;


            myfile << "\nGID is " << gid;

            int tileset_col;
            int tileset_row;



            gid--; // convert to something we can use with the 0 based system.

            tileset_col = (gid % NUM_TILESET_COLS);
            tileset_row = gid / NUM_TILESET_COLS;


            srcrect.x = getTilesetMargin() + (getTileSpacing() + 32) * tileset_col;
            srcrect.y = getTilesetMargin() + (getTileSpacing() + 32) * tileset_row;
            srcrect.w = getTileWidth();
            srcrect.h = getTileHeight();

            dstrect.x = col * getTileWidth();
            dstrect.y = row * getTileHeight();
            dstrect.w = getTileWidth();
            dstrect.h = getTileHeight();

            SDL_BlitSurface(img_tileset, &srcrect, img_map, &dstrect);
        }
    }
}


SDL_Surface* TMXLoader::getMapImage()
{
    return img_map;
}
