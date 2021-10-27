// ���ļ�Ϊ��Ҫ���л���SyntaxGenerator�ж�����ڲ�ʵ����Ҫ�����ע�����
// ע����Щ���Ա�boost::serialization�������л���Щ��
#ifndef GENERATOR_SYNTAXGENERATOR_SYNTAX_GENERATOR_CLASSES_REGISTER_H_
#define GENERATOR_SYNTAXGENERATOR_SYNTAX_GENERATOR_CLASSES_REGISTER_H_

#include "syntax_generator.h"

// ע�����л���Ҫʹ�õ���
BOOST_CLASS_EXPORT_GUID(
    frontend::generator::syntax_generator::
        ExportSyntaxGeneratorInsideTypeForSerialization::ProductionNodeId,
    "frontend::generator::syntax_generator::SyntaxGenerator::ProductionNodeId")
BOOST_CLASS_EXPORT_GUID(
    frontend::generator::syntax_generator::
        ExportSyntaxGeneratorInsideTypeForSerialization::ProcessFunctionClassId,
    "frontend::generator::syntax_generator::SyntaxGenerator::"
    "ProcessFunctionClassId")
BOOST_CLASS_EXPORT_GUID(frontend::generator::syntax_generator::
                            ExportSyntaxGeneratorInsideTypeForSerialization::
                                ProcessFunctionClassManagerType,
                        "frontend::generator::syntax_generator::"
                        "SyntaxGenerator::ProcessFunctionClassManagerType")
BOOST_CLASS_EXPORT_GUID(
    frontend::generator::syntax_generator::
        ExportSyntaxGeneratorInsideTypeForSerialization::ParsingTableType,
    "frontend::generator::syntax_generator::SyntaxGenerator::ParsingTableType")
BOOST_CLASS_EXPORT_GUID(
    frontend::generator::syntax_generator::
        ExportSyntaxGeneratorInsideTypeForSerialization::ParsingTableEntry,
    "frontend::generator::syntax_generator::SyntaxGenerator::ParsingTableEntry")
BOOST_CLASS_EXPORT_GUID(
    frontend::generator::syntax_generator::
        ExportSyntaxGeneratorInsideTypeForSerialization::ParsingTableEntryId,
    "frontend::generator::syntax_generator::SyntaxGenerator::"
    "ParsingTableEntryId")
BOOST_CLASS_EXPORT_GUID(
    frontend::generator::syntax_generator::
        ExportSyntaxGeneratorInsideTypeForSerialization::
            ParsingTableEntryActionAndReductDataInterface,
    "frontend::generator::syntax_generator::SyntaxGenerator::"
    "ParsingTableEntryActionAndReductDataInterface")
BOOST_CLASS_EXPORT_GUID(frontend::generator::syntax_generator::
                            ExportSyntaxGeneratorInsideTypeForSerialization::
                                ParsingTableEntryShiftAttachedData,
                        "frontend::generator::syntax_generator::"
                        "SyntaxGenerator::ParsingTableEntryShiftAttachedData")
BOOST_CLASS_EXPORT_GUID(frontend::generator::syntax_generator::
                            ExportSyntaxGeneratorInsideTypeForSerialization::
                                ParsingTableEntryReductAttachedData,
                        "frontend::generator::syntax_generator::"
                        "SyntaxGenerator::ParsingTableEntryReductAttachedData")
BOOST_CLASS_EXPORT_GUID(
    frontend::generator::syntax_generator::
        ExportSyntaxGeneratorInsideTypeForSerialization::
            ParsingTableEntryShiftReductAttachedData,
    "frontend::generator::syntax_generator::SyntaxGenerator::"
    "ParsingTableEntryShiftReductAttachedData")

#endif  // !GENERATOR_SYNTAXGENERATOR_SYNTAX_GENERATOR_CLASSES_REGISTER_H_
