#pragma once

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include "BladeInterpolator.h"
#include "DXFDocument.h"
#include "DXFFileWriter.h"
#include "DXFPoint3D.h"
#include "DXFColor.h"
#include "DXFEntityFactory.h"
#include "DXFFormatter.h"


/**
 * @brief Class to create a 3D DXF model of a blade using BladeInterpolator data
 * 
 */
class DXFBlade3D
{
	private:

		/**
		 * @brief BladeInterpolator instance providing access to interpolator
		 */
		std::unique_ptr<BladeInterpolator> bladeInterpolator;

		/**
		 * @brief BladeGeometrySection instance providing blade section data
		 */
		std::vector<std::unique_ptr<BladeGeometrySection>> bladeSections;
		
		/**
		 * @brief 
		 */
		std::string dxfFilePath;

		/**
		 * @brief strore of all 3D points for DXF document
		 */
		std::vector<DXFPoint3D> dxfPointList;
		

	public:
		
		explicit DXFBlade3D(std::unique_ptr<BladeInterpolator> bladeInterpolator);
		
		~DXFBlade3D();
		
		/**
		 * @brief Create DXF Document with  DXF entities and airfoil data 
		 * 
		 */
		void createDXFModel();

		/**
		 * @brief Write the DXF Document to file
		 * 
		 */
		void writeDXFFile() const;

		/**
		 * @brief Write the section data to one string for text file writing
		 * @return string with section data
		 */
		std::string dataToString() const;
		
};

