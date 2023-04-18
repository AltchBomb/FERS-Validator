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
        if (argc == 2) {
            mode = argv[1];
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
        std::string file_path = "FERSXML-example/CleanSingleTarget.fersxml";

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

            } else {
                std::cout << "XML document is invalid" << std::endl;
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


/*
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
parser.setExternalSchemaLocation("./schema.xsd", "schema.xsd");

// Establish a DOMDocument object and parse the input FERSXML file:
// A pointer variable of type xerces::DOMDocument is created. parser.parseURI("input.xml") returns a pointer.
xercesc::DOMDocument* doc = parser.parserURI("sample1.xml");

// Create a schema validator object and specify the location of the schema:
// A pointer variable of type xerces::SchemaValidator is created.
xercesc::SchemaValidator* validator = new xercesc::SchemaValidator();
validator->setExternalSchemaLocation("./schema.xsd", "schema.xsd");

// Using the SchemaValidator to validate the DOMDocument object:
// try block used to catch any exceptions that might be thrown while the validation process is running.
try {
    validator->validate(*doc);
    std::cout << "XML is valid." << std::endl;
} catch (const xercesc::XMLException& e) {
    std::cerr << "Error: " << xercesc::XMLString::transcode(e.getMessage()) << std::endl;
} catch (const xercesc::DOMException& e) {
    std::cerr << "Error: " << xercesc::XMLString::transcode(e.getMessage()) << std::endl;
} catch (const xercesc::SAXParseException& e) {
    std::cerr << "Error: " << xercesc::XMLString::transcode(e.getMessage()) << " at line " << e.getLineNumber() << ", column " << e.getColumnNumber() << std::endl;
} catch (...) {
    std::cerr << "Error: unknown exception" << std::endl;
}

// Cleanup memory and terminate the Xerces-C++ library from running, and free up memory allocation:
parser.resetDocumentPool();
validator->~SchemaValidator();
xercesc::XMLPlatformUtils::Terminate();

*/