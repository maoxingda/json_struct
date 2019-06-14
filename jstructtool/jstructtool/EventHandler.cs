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
                    cbt.CommandLine = "json2cxxstructHelper.exe \"%(FullPath)\" \".\\GeneratedFiles\\%(Filename).h\"";
                    cbt.Description = "json2cxxstructHelper.exe%27ing %(Identity)...";
                    cbt.Outputs     = ".\\GeneratedFiles\\%(Filename).h";
                }
            }
        }

        private DTE2                        _applicationObject;
        private EnvDTE.ProjectItemsEvents   _projectItemsEvents;
    }
}
