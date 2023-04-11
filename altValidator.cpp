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

class MyErrorHandler : public ErrorHandler
{
public:
    void warning(const SAXParseException& exc)
    {
        cerr << "Warning at line " << exc.getLineNumber()
             << ", column " << exc.getColumnNumber()
             << ": " << XMLString::transcode(exc.getMessage()) << endl;
    }

    void error(const SAXParseException& exc)
    {
        cerr << "Error at line " << exc.getLineNumber()
             << ", column " << exc.getColumnNumber()
             << ": " << XMLString::transcode(exc.getMessage()) << endl;
    }

    void fatalError(const SAXParseException& exc)
    {
        cerr << "Fatal Error at line " << exc.getLineNumber()
             << ", column " << exc.getColumnNumber()
             << ": " << XMLString::transcode(exc.getMessage()) << endl;
    }

    void resetErrors()
    {
        // reset the error count/nature
    }
};

int main(int argc, char* argv[])
{
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
        parser.setExternalNoNamespaceSchemaLocation("schema-test.xsd");

        // Creating SchemaValidator object and setting it up with parser:
        MyErrorHandler errorHandler;
        parser.setErrorHandler(&errorHandler);
        SchemaValidator validator(parser.getValidationContext());
        validator.setErrorHandler(&errorHandler);

        // Establish a DOMDocument object and parse the input FERSXML file:
        parser.parse("generic-fers-xml.xml");

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

    } catch (const xercesc::DOMException& e) {
        std::cerr << "Error parsing XML document: " << e.getMessage() << std::endl;
        return 1;

    } catch (const xercesc::SAXParseException& e) {
        std::cerr << "Error parsing XML document: " << e.getMessage() << " at line " << e.getLineNumber() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "Error parsing XML document: unknown exception" << std::endl;
        return 1;
    }

};