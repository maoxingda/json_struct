using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using EnvDTE;
using EnvDTE80;
using Microsoft.Win32;
using Microsoft.VisualStudio.VCProjectEngine;
using System.Windows.Forms;

namespace jstructwizd
{
    public class AddNewItem : IDTWizard
    {
        private DTE2 _applicationObject            = null;
        private string template_file_name_src_jst  = null;
        private string template_file_name_src_json = null;
        private string template_file_name_dst      = null;

        public void Execute(object Application,
            int hwndOwner,
            ref object[] contextParams,
            ref object[] customParams,
            ref EnvDTE.wizardResult retval)
        {
            try
            {
                _applicationObject = Application as DTE2;

                if (null == template_file_name_src_jst || null == template_file_name_src_json)
                {
                    RegistryKey vs2010 = Registry.CurrentUser.OpenSubKey("software\\jstructtool");

                    template_file_name_src_jst  = vs2010.GetValue("AppFolder").ToString() + "\\inc\\template.jst";
                    template_file_name_src_json = vs2010.GetValue("AppFolder").ToString() + "\\inc\\template.json";

                    vs2010.Close();
                }

                foreach (SelectedItem si in _applicationObject.SelectedItems)
                {
                    VCFilter filter = si.ProjectItem.Object as VCFilter;
                    VCProject proj = filter.project as VCProject;

                    template_file_name_dst = contextParams[4] as string;

                    if (!template_file_name_dst.EndsWith(".json"))
                    {
                        template_file_name_dst += ".jst";

                        File.Copy(template_file_name_src_jst, template_file_name_dst);
                    }
                    else
                    {
                        File.Copy(template_file_name_src_json, template_file_name_dst);
                    }

                    if (filter.CanAddFile(template_file_name_dst))
                    {
                        if (template_file_name_dst.EndsWith(".jst"))
                        {
                            string text = null;

                            using (StreamReader sr = new StreamReader(template_file_name_dst))
                            {
                                text = sr.ReadToEnd();

                                text = text.Replace("%struct_name%", Path.GetFileNameWithoutExtension(template_file_name_dst));
                            }

                            using (StreamWriter sw = new StreamWriter(template_file_name_dst))
                            {
                                sw.Write(text);
                            }
                        }

                        filter.AddFile(template_file_name_dst);

                        // open added file
                        _applicationObject.ItemOperations.OpenFile(template_file_name_dst);

                        if (template_file_name_dst.EndsWith(".json"))
                        {
                            template_file_name_dst = template_file_name_dst.Remove(template_file_name_dst.LastIndexOf(".json")) + ".jst";

                            filter.AddFile(template_file_name_dst);
                        }
                    }
                }
            }
            catch (IOException e)
            {
                MessageBox.Show(template_file_name_dst + " existed!");
            }
            catch (Exception e)
            {
            }
        }
    }
}
