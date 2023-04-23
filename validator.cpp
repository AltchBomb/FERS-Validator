// FERS input Validator Function
// Script written by Michael Altshuler 
// University of Cape Town: ALTMIC003

// The following implementation makes use of the Document Object Model (DOM) and a custom XSD Schema.

#include <iostream>
#include <fstream>
#include <string>

// Including necessary Xerces-C++ header files:
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/validators/schema/SchemaValidator.hpp>
#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/dom/DOMLSSerializer.hpp>
#include <xercesc/dom/DOMLSOutput.hpp>

using namespace xercesc;
using namespace std;

// Error handler that catches validation errors and further provides information about the errors.
class MyErrorHandler : public ErrorHandler {
public:
    void warning(const SAXParseException& e) {
        cerr << "Warning: " << XMLString::transcode(e.getMessage()) << " at line " << e.getLineNumber() << ", column " << e.getColumnNumber() << endl;
    }

    void error(const SAXParseException& e) {
        cerr << "Error: " << XMLString::transcode(e.getMessage()) << " at line " << e.getLineNumber() << ", column " << e.getColumnNumber() << endl;
    }

    void fatalError(const SAXParseException& e) {
        cerr << "Fatal Error: " << XMLString::transcode(e.getMessage()) << " at line " << e.getLineNumber() << ", column " << e.getColumnNumber() << endl;
    }

    void resetErrors() {}
};

int main(int argc, char* argv[]) {

    try {

        // Check if the user provided the mode argument
        std::string mode = "verbose";
        if (argc == 3) {
            mode = argv[2];
            if (mode != "verbose" && mode != "non-verbose") {
                std::cerr << "Invalid mode argument. Please provide 'verbose' or 'non-verbose'." << std::endl;
                return 1;
            }
        } 

        // Initializing Xerces-C++ library:
        xercesc::XMLPlatformUtils::Initialize();

        // A XercesDOMParser object is set along with its features:
        xercesc::XercesDOMParser parser;
        // Enables validation during parsing.
        parser.setValidationScheme(xercesc::XercesDOMParser::Val_Always);
        // Enables validation against an XSD Schema file.
        parser.setDoNamespaces(true);
        // Enables full schema constraint checking during validation.
        parser.setDoSchema(true);

        // Set the custom XSD file for the parser by specifyng the XSD file path and XSD file name
        parser.setExternalNoNamespaceSchemaLocation("/Users/michaelaltshuler/Documents/5th Year/EEE4022F:Thesis/FERS Features/FERS Validator/FERS-schema/fers-xml.xsd");

        // error handler instance created and set on the parser.
        MyErrorHandler errorHandler;
        parser.setErrorHandler(&errorHandler);

        // File to be parsed
        std::string file_path = argv[1];

        // Establish a DOMDocument object and parse the input FERSXML file:
        parser.parse(file_path.c_str());

        cout<< endl;

        if(parser.getErrorCount() == 0) {
                std::cout << "User is in " << mode << " mode." << std::endl;
                cout << endl;

                std::cout << "XML document is valid" << std::endl;

                // Command to run xml_validator_output with mode and file_path as argument
                std::string command = "./xml_validator_output " + file_path + " " + mode;
                // Run the command using system() function
                int result = std::system(command.c_str());

                if (result == -1) {
                    std::cerr << "Failed to run xml_validator_output." << std::endl;
                    return 1;
                }

                std::cout << endl;

                // Prompting user whether they want to run the kml_visualiser
                char run_kml_visualiser;
                char custom_coordinates;
                double referenceLatitude = -33.9545;
                double referenceLongitude = 18.4563;
                double referenceAltitude = 0;
                string outputName;

                std::cout << "Do you want to run the kml_visualiser program? (y/n): ";
                std::cin >> run_kml_visualiser;

                if (run_kml_visualiser == 'y' || run_kml_visualiser == 'Y') {
                    std::cout << "Name a KML output file: ";
                    std::cin >> outputName;

                    std::cout << "Do you want to enter custom referenceLatitude, referenceLongitude, and referenceAltitude? (y/n): ";
                    std::cin >> custom_coordinates;
                    std:: cout << "Default coordinates set to University of Cape Town: <133.9577° S, 18.4612° E , 0>" << std::endl;

                    if (custom_coordinates == 'y' || custom_coordinates == 'Y') {
                        std::cout << "Enter referenceLatitude (default: -33.9545): ";
                        std::cin >> referenceLatitude;

                        std::cout << "Enter referenceLongitude (default: 18.4563): ";
                        std::cin >> referenceLongitude;

                        std::cout << "Enter referenceAltitude (default: 0): ";
                        std::cin >> referenceAltitude;
                    }

                    std::string kml_visualiser_command = "./kml_visualiser";

                    if (custom_coordinates == 'y' || custom_coordinates == 'Y') {
                        kml_visualiser_command += " " + file_path + " " + outputName + ".kml" + " " + std::to_string(referenceLatitude) + " " + std::to_string(referenceLongitude) + " " + std::to_string(referenceAltitude);
                        
                        int kml_visualiser_result = std::system(kml_visualiser_command.c_str());

                        if (kml_visualiser_result == -1) {
                            std::cerr << "Failed to run kml_visualiser." << std::endl;
                            return 1;
                        }
                    } else {
                        kml_visualiser_command += " " + file_path + " " + outputName + ".kml";
                        int kml_visualiser_result = std::system(kml_visualiser_command.c_str());

                        if (kml_visualiser_result == -1) {
                            std::cerr << "Failed to run kml_visualiser." << std::endl;
                            return 1;
                        }
                    }

                    std::cout << outputName + ".kml" + " outputted to current working directory." << std::endl; 

                } else if (run_kml_visualiser != 'n' && run_kml_visualiser != 'N') {
                    std::cerr << "Invalid input. Please enter 'y' for yes or 'n' for no." << std::endl;
                    return 1;
                }

        }

            // Function call deallocates memory associated with he parsed FERSXML document.
            //parser.resetDocumentPool();
            // Shuts down and deallocates any memory associated with the Xerces-C++ library.
            //xercesc::XMLPlatformUtils::Terminate();
            return 0;

    } catch (const xercesc::XMLException& e) {
        std::cerr << "Error parsing XML document: " << e.getMessage() << std::endl;
        return 1;

    }  catch (const xercesc::DOMException& e) {
        std::cerr << "Error parsing XML document: " << e.getMessage() << std::endl;
        return 1;

    } catch (const xercesc::SAXParseException& e) {
        std::cerr << "Error parsing XML document: " << e.getMessage() << " at line " << e.getLineNumber() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "Error parsing XML document: unknown exception" << std::endl;
        return 1;
    }
}