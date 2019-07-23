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
    public class ItemEventHandler
    {
        public ItemEventHandler(DTE2 applicationObject)
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

                if (!file.Name.EndsWith(".jst")) return;

                //// open added file
                //projectItem.Open();

                foreach (VCFileConfiguration fc in (IVCCollection)file.FileConfigurations)
                {
                    VCCustomBuildTool cbt = fc.Tool as VCCustomBuildTool;

                    cbt.CommandLine = "jstructcompiler --multi_build=off --my_doc_path=\"$(VisualStudioDir)\" --h_out --input_file=\"%(FullPath)\" --output_file=\"$(ProjectDir)mjst\\%(Filename).h\"";
                    cbt.Description = "jstructcompiler%27ing %(Identity)...";
                    cbt.Outputs     = "$(ProjectDir)mjst\\%(Filename).h";
                }

                VCProject           proj;
                IVCCollection       cfgs, tools;
                VCLinkerTool        linktool;
                VCCLCompilerTool    comptool;

                proj = projectItem.ContainingProject.Object  as VCProject;
                cfgs = proj.Configurations                   as IVCCollection;

                string incpath = "$(VisualStudioDir)\\Addins\\inc;";
                string libpath = "$(VisualStudioDir)\\Addins\\lib;";

                foreach (VCConfiguration cfg in cfgs as IVCCollection)
                {
                    tools       = cfg.Tools                         as IVCCollection;
                    comptool    = tools.Item("VCCLCompilerTool")    as VCCLCompilerTool;
                    linktool    = tools.Item("VCLinkerTool")        as VCLinkerTool;

                    StringBuilder sbcomp = new StringBuilder(comptool.AdditionalIncludeDirectories);
                    StringBuilder sblink = new StringBuilder(linktool.AdditionalLibraryDirectories);

                    if (!sbcomp.ToString().Contains("$(VisualStudioDir)"))
                    {
                        sbcomp.Insert(0, incpath + ".\\mjst;");

                        comptool.AdditionalIncludeDirectories = sbcomp.ToString();
                    }

                    if (!sblink.ToString().Contains("$(VisualStudioDir)"))
                    {
                        sblink.Insert(0, libpath);

                        linktool.AdditionalLibraryDirectories = sblink.ToString();
                    }
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
