// FERS input Validator Function
// Script written by Michael Altshuler 
// University of Cape Town: ALTMIC003

// The following implementation makes use of the Document Object Model (DOM) and a custom XSD Schema.

#include <iostream>

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

int main(int argc, char* argv[]) {

    try {

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
       parser.setExternalNoNamespaceSchemaLocation("schema.xsd");

        // Establish a DOMDocument object and parse the input FERSXML file:
        parser.parse("sample1.xml");

        if(parser.getErrorCount() == 0) {
                std::cout << "XML document is valid" << std::endl;

                // Specify the root element of the parsed XML document.
                DOMElement* rootElement = parser.getDocument()->getDocumentElement();

                // Generating a DOMLSSerializer object.
                DOMImplementation* domImpl = DOMImplementationRegistry::getDOMImplementation(XMLString::transcode("LS"));
                DOMLSSerializer* serializer = ((DOMImplementationLS*)domImpl)->createLSSerializer();

                // Set the serializer up to produce data in a readable manner.
                DOMConfiguration* domConfig = serializer->getDomConfig();
                domConfig->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);

                // Serialize the DOM tree to a string and output to console if the XML document is valid.
                std::string xmlString = XMLString::transcode(serializer->writeToString(rootElement));
                std::cout << xmlString << std::endl;

                // Deallocate the serializer object.
                serializer->release();

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