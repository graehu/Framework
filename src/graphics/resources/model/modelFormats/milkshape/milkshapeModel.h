#ifndef MILKSHAPEMODEL_H
#define MILKSHAPEMODEL_H

#include "../../model.h"

class milkshapeModel : public model
{
	public:
		milkshapeModel();
		virtual ~milkshapeModel();

		virtual bool loadModelData( const char *filename );
};

#endif//MILKSHAPEMODEL_H
