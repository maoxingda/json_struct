using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using EnvDTE;
using EnvDTE80;
using Microsoft.VisualStudio.VCProjectEngine;

namespace jstructtool
{
    public class EventHandler
    {
        public EventHandler(DTE2 applicationObject)
        {
            _applicationObject  = applicationObject;

            Events2 events2     = _applicationObject.Events as Events2;

            _projectItemsEvents = (ProjectItemsEvents)events2.ProjectItemsEvents;
            _projectItemsEvents.ItemAdded += new _dispProjectItemsEvents_ItemAddedEventHandler(ProjectItemsEvents_ItemAdded);
        }

        public void ProjectItemsEvents_ItemAdded(ProjectItem projectItem)
        {
            VCFile file = projectItem.Object as VCFile;

            if (null == file) return;

            if (!file.Name.EndsWith(".json.h")) return;

            foreach (VCFileConfiguration fc in (IVCCollection)file.FileConfigurations)
            {
                VCCustomBuildTool cbt = fc.Tool as VCCustomBuildTool;

                if (null != cbt)
                {
                    cbt.CommandLine = "jstructcompiler --multi_build off --h_out --output_path $(ProjectDir)mjst\\ --input_files %(FullPath)";
                    cbt.Description = "jstructcompiler%27ing %(Identity)...";
                    cbt.Outputs     = "$(ProjectDir)mjst\\%(Filename)";
                }
            }

            int length = 0;
            using (StreamReader sr = new StreamReader(file.FullPath))
            {
                length = sr.ReadToEnd().Length;

                sr.Close();
            }

            if (0 != length) return;

            using (StreamWriter sw = new StreamWriter(file.FullPath))
            {
                sw.WriteLine("#pragma once");
                sw.WriteLine("#include <jstruct.h>");

                sw.WriteLine();
                sw.WriteLine();

                sw.WriteLine("/*bracket indicate optional");
                sw.WriteLine("struct struct_name");
                sw.WriteLine("{");

                sw.WriteLine("    REQUIRED|OPTIONAL                     BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY [ALIAS(alias_name)]                   field_type field_name;");
                sw.WriteLine("    REQUIRED|OPTIONAL                     [ALIAS(alias_name)]                   BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY field_type field_name;");

                sw.WriteLine("    BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY REQUIRED|OPTIONAL                     [ALIAS(alias_name)]                   field_type field_name;");
                sw.WriteLine("    BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY [ALIAS(alias_name)]                   REQUIRED|OPTIONAL                     field_type field_name;");

                sw.WriteLine("    [ALIAS(alias_name)]                   REQUIRED|OPTIONAL                     BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY field_type field_name;");
                sw.WriteLine("    [ALIAS(alias_name)]                   BASIC|BASIC_ARRAY|CUSTOM|CUSTOM_ARRAY REQUIRED|OPTIONAL                     field_type field_name;");

                sw.WriteLine("    ...");

                sw.WriteLine("};");
                sw.WriteLine("*/");

                sw.Close();
            }
        }

        private DTE2                        _applicationObject;
        private EnvDTE.ProjectItemsEvents   _projectItemsEvents;
    }
}
