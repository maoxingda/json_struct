using System;
using System.IO;
using System.Xml;
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
        private DTE2 _applicationObject        = null;
        private string template_file_name_src  = null;
        private string template_file_name_dst  = null;
        private string add_output_file         = null;
        private string app_folder              = null;

        public void Execute(object Application,
            int hwndOwner,
            ref object[] contextParams,
            ref object[] customParams,
            ref EnvDTE.wizardResult retval)
        {
            try
            {
                _applicationObject = Application as DTE2;

                if (null == template_file_name_src)
                {
                    RegistryKey vs2010 = Registry.CurrentUser.OpenSubKey("software\\jstructtool");

                    app_folder = vs2010.GetValue("AppFolder").ToString();

                    template_file_name_src = app_folder + "\\inc\\template.jst";

                    vs2010.Close();
                }

                foreach (SelectedItem si in _applicationObject.SelectedItems)
                {
                    VCFilter filter = si.ProjectItem.Object as VCFilter;
                    VCProject proj = filter.project as VCProject;

                    template_file_name_dst = contextParams[4] as string;

                    template_file_name_dst += ".jst";

                    File.Copy(template_file_name_src, template_file_name_dst);

                    if (filter.CanAddFile(template_file_name_dst))
                    {
                        string text = null;

                        using (StreamReader sr = new StreamReader(template_file_name_dst))
                        {
                            text = sr.ReadToEnd();

                            text = text.Replace("%ns%", Path.GetFileNameWithoutExtension(template_file_name_dst));
                        }

                        using (StreamWriter sw = new StreamWriter(template_file_name_dst))
                        {
                            sw.Write(text);
                        }

                        filter.AddFile(template_file_name_dst);

                        if (null == add_output_file)
                        {
                            XmlDocument doc = new XmlDocument();

                            doc.Load(app_folder + "\\debugconf.xml");

                            XmlNode add_o_file = doc.SelectSingleNode("/debug/add_output_file");

                            add_output_file = add_o_file.InnerText;
                        }

                        if ("true" == add_output_file) filter.AddFile(proj.ProjectDirectory + "\\mjst\\" + Path.GetFileNameWithoutExtension(template_file_name_dst) + ".h");

                        // open added file
                        _applicationObject.ItemOperations.OpenFile(template_file_name_dst);
                    }
                }
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString());
            }
        }
    }
}
