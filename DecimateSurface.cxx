#include <vtkPolyData.h>
#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
#include <vtkDecimatePro.h>

#include "utils/io.h"
#include "utils/itkCommandLineArgumentParser.h"

int main(int argc, char** argv)
{
  itk::CommandLineArgumentParser::Pointer parser = itk::CommandLineArgumentParser::New();
  parser->SetCommandLineArguments(argc, argv);

  std::string inputFile;
  parser->GetCommandLineArgument("-input", inputFile);

  std::string outputFile;
  parser->GetCommandLineArgument("-output", outputFile);

  unsigned int numberOfPoints = 100000;
  parser->GetCommandLineArgument("-points", numberOfPoints);

  double relaxation = 0.2;
  parser->GetCommandLineArgument("-relaxation", relaxation);

  unsigned int iterations = 20;
  parser->GetCommandLineArgument("-iteration", iterations);

  std::cout << std::endl;
  std::cout << "parameters" << std::endl;
  std::cout << "     points " << numberOfPoints << std::endl;
  std::cout << " relaxation " << relaxation << std::endl;
  std::cout << " iterations " << iterations << std::endl;
  std::cout << std::endl;

  // read input polydata
  typedef vtkSmartPointer<vtkPolyData> vtkPolyData;
  vtkPolyData surface = vtkPolyData::New();

  if (!readVTKPolydata(surface, inputFile)) {
      return EXIT_FAILURE;
  }

  std::cout << "input surface " << inputFile << std::endl;
  std::cout << " number of cells " << surface->GetNumberOfCells() << std::endl;
  std::cout << "number of points " << surface->GetNumberOfPoints() << std::endl;
  std::cout << std::endl;

  // decimate surface
  double reduction = 1 - numberOfPoints / (double) surface->GetNumberOfPoints();
  std::cout << "reduction to decimate surface " << reduction << std::endl;

  vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
  decimate->SetInputData(surface);
  decimate->SetTargetReduction(reduction);
  decimate->SetPreserveTopology(true);
  decimate->SetSplitting(false);
  decimate->Update();

  typedef vtkSmartPointer<vtkSmoothPolyDataFilter> SmoothPolyData;
  SmoothPolyData smoother = SmoothPolyData::New();
  smoother->SetInputData(decimate->GetOutput());
  smoother->SetNumberOfIterations(iterations);
  smoother->SetRelaxationFactor(relaxation);
  try {
    smoother->Update();
  }
  catch (itk::ExceptionObject& excep) {
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }

  typedef vtkSmartPointer<vtkPolyDataNormals> PolyDataNormals;
  PolyDataNormals normals = PolyDataNormals::New();
  normals->SetInputData(smoother->GetOutput());
  normals->AutoOrientNormalsOn();
  normals->FlipNormalsOff();
  normals->ConsistencyOn();
  normals->ComputeCellNormalsOff();
  normals->SplittingOff();
  try {
    normals->Update();
  }
  catch (itk::ExceptionObject& excep) {
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
  }

  vtkPolyData output = normals->GetOutput();

  // write polydata to the file
  if (!writeVTKPolydata(output, outputFile)) {
    return EXIT_FAILURE;
  }
  
  std::cout << "output mesh " << outputFile << std::endl;
  std::cout << " number of cells " << output->GetNumberOfCells() << std::endl;
  std::cout << "number of points " << output->GetNumberOfPoints() << std::endl;
  std::cout << std::endl;

  return EXIT_SUCCESS;
}

