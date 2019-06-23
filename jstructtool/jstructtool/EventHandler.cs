using System;
using System.IO;
using Microsoft.Win32;
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
            try
            {
                VCFile file = projectItem.Object as VCFile;

                if (null == file) return;

                if (!file.Name.EndsWith(".json.h")) return;

                foreach (VCFileConfiguration fc in (IVCCollection)file.FileConfigurations)
                {
                    VCCustomBuildTool cbt = fc.Tool as VCCustomBuildTool;

                    if (null != cbt)
                    {
                        cbt.CommandLine = "jstructcompiler --multi_build=off --h_out --input_file=\"%(FullPath)\" --output_file=\"$(ProjectDir)mjst\\%(Filename).h\"";
                        cbt.Description = "jstructcompiler%27ing %(Identity)...";
                        cbt.Outputs = "$(ProjectDir)mjst\\%(Filename).h";
                    }
                }

                int length = 0;
                using (StreamReader sr = new StreamReader(file.FullPath))
                {
                    length = sr.ReadToEnd().Length;

                    sr.Close();
                }

                if (0 != length) return;

                string template_file_name = "";
                string template_file_text = "";

                RegistryKey vs2010 = Registry.CurrentUser.OpenSubKey("SOFTWARE\\Microsoft\\VisualStudio\\10.0_Config");

                if (null != vs2010)
                {
                    template_file_name = vs2010.GetValue("ShellFolder").ToString() + "\\VC\\include\\template.json.h";

                    using (StreamReader sr = new StreamReader(template_file_name))
                    {
                        template_file_text = sr.ReadToEnd();
                    }

                    vs2010.Close();
                }

                using (StreamWriter sw = new StreamWriter(file.FullPath))
                {
                    if (0 != template_file_text.Length)
                    {
                        sw.Write(template_file_text);
                    }

                    sw.Close();
                }
            }
            catch (Exception e)
            {
            }
        }

        private DTE2                        _applicationObject;
        private EnvDTE.ProjectItemsEvents   _projectItemsEvents;
    }
}
