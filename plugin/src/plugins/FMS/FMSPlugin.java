package plugins.FMS;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLDecoder;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import freenet.clients.http.PageMaker;
import freenet.l10n.BaseL10n.LANGUAGE;
import freenet.pluginmanager.FredPlugin;
import freenet.pluginmanager.FredPluginHTTP;
import freenet.pluginmanager.FredPluginL10n;
import freenet.pluginmanager.FredPluginRealVersioned;
import freenet.pluginmanager.FredPluginThreadless;
import freenet.pluginmanager.FredPluginVersioned;
import freenet.pluginmanager.PluginHTTPException;
import freenet.pluginmanager.PluginRespirator;
import freenet.pluginmanager.RedirectPluginHTTPException;
import freenet.support.api.HTTPRequest;
import freenet.support.io.Closer;

public class FMSPlugin implements FredPlugin, FredPluginThreadless, FredPluginHTTP, FredPluginRealVersioned, FredPluginVersioned, FredPluginL10n {

	private static final String ResourcePath="plugins/FMS/resources/";
	private static final int BUFFER = 2048;
	PageMaker pagemaker;

	/** Shut down the plugin. */
	@Override
	public void terminate()
	{
		StopFMS();
		pagemaker.removeNavigationCategory("FMS");
	}

	/** Run the plugin. Called after node startup. Should be able to access
	 * queue etc at this point. Plugins which do not implement
	 * FredPluginThreadless will be terminated after they return from this
	 * function. Threadless plugins will not terminate until they are
	 * explicitly unloaded. */
	@Override
	public void runPlugin(PluginRespirator pr)
	{
		extractFMSResources();

		StartFMS();
		pagemaker = pr.getPageMaker();
        pagemaker.removeNavigationCategory("FMS");
        pagemaker.addNavigationCategory("/plugins/plugins.FMS.FMSPlugin","FMS","Freenet Message System",this);
        pagemaker.addNavigationLink("FMS", "/plugins/plugins.FMS.FMSPlugin" , "Freenet Message System", "Freenet Message System", true, null, this);
        pagemaker.addNavigationLink("FMS","/USK@xedmmitRTj9-PXJxoPbD7RY1gf9pKi0OcsRmjNPPIU4,AzFWTYV~9-I~eXis14tIkJ4XkF17gIgZrB294LjFXjc,AQACAAE/fmsguide/3/","The Unofficial Guide to FMS","The Unofficial Guide to FMS",true,null,this);
	}

	@Override
	public long getRealVersion()
	{
		return VersionLong();
	}

	@Override
	public String getVersion()
	{
		return VersionString();
	}

	// JNI methods
	public native void StartFMS();
	//public native void RestartFMS();
	public native void StopFMS();
	public native String VersionString();
	public native long VersionLong();
	public native long GetHTTPListenPort();

	private static final boolean tryLoadResource(File f, URL resource)
		throws FileNotFoundException, UnsatisfiedLinkError {
		InputStream is;
		try {
			is = resource.openStream();
		} catch(IOException e) {
			f.delete();
			throw new FileNotFoundException();
		}

		FileOutputStream fos = null;
		try {
			f.deleteOnExit();
			fos = new FileOutputStream(f);
			byte[] buf = new byte[4096 * 1024];
			int read;
			while((read = is.read(buf)) > 0) {
				fos.write(buf, 0, read);
			}
			fos.close();
			fos = null;
			System.load(f.getAbsolutePath());
			return true;
		} catch(IOException e) {
		} catch(UnsatisfiedLinkError ule) {
			// likely to be "noexec"
			if(ule.toString().toLowerCase().indexOf("not permitted") == -1)
				throw ule;
		} finally {
			Closer.close(fos);
			f.delete();
		}

		return false;
	}

	static
	{

		String resourcepath=FMSPlugin.class.getPackage().getName().replace('.', '/')+"/FMS-"+getPlatform()+"-"+getArchitechture()+"."+getSuffix();
		URL resource=FMSPlugin.class.getClassLoader().getResource(resourcepath);
		File temp=null;

		try
		{
			temp=File.createTempFile("FMS","bin.tmp");
			tryLoadResource(temp,resource);
		}
		catch(IOException e)
		{
		}
		finally
		{
			if(temp!=null)
			{
				temp.delete();
			}
		}

	}

	private static String getPlatform()
	{
		boolean isWindows = (System.getProperty("os.name").toLowerCase().indexOf("windows") != -1);
		boolean isLinux = (System.getProperty("os.name").toLowerCase().indexOf("linux") != -1);
		boolean isFreebsd = (System.getProperty("os.name").toLowerCase().indexOf("freebsd") != -1);
		boolean isMacOS = (System.getProperty("os.name").toLowerCase().indexOf("mac os x") != -1);
		if(isWindows)
			return "win"; // The convention on Windows
		if(isLinux)
			return "linux"; // The convention on linux...
		if(isFreebsd)
			return "freebsd"; // The convention on freebsd...
		if(isMacOS)
			return "osx"; // The convention on Mac OS X...
		throw new RuntimeException("Dont know library name for os type '" + System.getProperty("os.name") + '\'');
	}

	private static String getArchitechture()
	{
		String arch;
		if(System.getProperty("os.arch").toLowerCase().matches("(i?[x0-9]86_64|amd64)")) {
			arch = "amd64";
		} else if(System.getProperty("os.arch").toLowerCase().matches("(ppc)")) {
			arch = "ppc";
		} else {
			// We want to try the i386 libraries if the architecture is unknown
			arch = "i386";
		}
		return arch;
	}

	private static String getSuffix()
	{
		boolean isWindows = System.getProperty("os.name").toLowerCase().indexOf("windows") != -1;
		boolean isMacOS = (System.getProperty("os.name").toLowerCase().indexOf("mac os x") != -1);
		if(isWindows)
			return "dll";
		else if(isMacOS)
			return "jnilib";
		else
			return "so";
	}


    public String handleHTTPGet(HTTPRequest request) throws PluginHTTPException{
        // request via URL
        //return request(request.getParam("search"),request.getIntParam("set"));
        String host="";
        int colonpos=request.getHeader("host").lastIndexOf(":");
        if(colonpos>=0)
        {
			host=request.getHeader("host").substring(0,colonpos);
		}
		else
		{
			host=request.getHeader("host");
		}
        throw new RedirectPluginHTTPException("Freenet Message System","http://"+host+":"+Long.toString(GetHTTPListenPort())+"/");
    }


    public String handleHTTPPost(HTTPRequest request) throws PluginHTTPException{
        return null;
    }
    
    private void extractFMSResources()
    {
		try
		{
			String name=new File(URLDecoder.decode(getClass().getResource("").getPath(), "UTF-8")).getParentFile().getParent();
			System.out.println("Name1="+name);
			if(name.length()>0 && name.substring(name.length()-1).equals("!"))
			{
				name=name.substring(0,name.length()-1);
			}
			System.out.println("Filename="+name);
			name=name.replace("file:/","");
			name=name.replace("file:\\","");
			name=name.replace('/',File.separatorChar);
			unzipArchive(name);
		}
		catch(Exception e)
		{
			System.out.println(e.getMessage());
		}
	}

	private static void unzipArchive(String filename) throws FileNotFoundException, IOException {
		FileInputStream fis = new FileInputStream(filename);
		ZipInputStream zis = new ZipInputStream(new BufferedInputStream(fis));

		ZipEntry entry;
		while((entry = zis.getNextEntry()) != null) {
			if(entry.getName().indexOf(ResourcePath)==0)
			{
				String name="FMS/"+entry.getName().substring(ResourcePath.length());
				if ( entry.isDirectory() ) {
					System.out.println("Creating "+name);
					createDir(name);
				} else {
					System.out.println("Extracting "+name);
					extractFile(zis, entry);
				}
			}
		}
		zis.close();
	}

	private static void extractFile(ZipInputStream zis, ZipEntry entry) throws FileNotFoundException, IOException {
		BufferedOutputStream dest;
		int count;
		byte data[] = new byte[BUFFER];
		// write the files to the disk
		FileOutputStream fos = new FileOutputStream("FMS/"+entry.getName().substring(ResourcePath.length()));
		dest = new BufferedOutputStream(fos, BUFFER);
		while ((count = zis.read(data, 0, BUFFER))!= -1) {
			dest.write(data, 0, count);
		}
		dest.flush();
		dest.close();
	}

	private static void createDir(String name) {
		File file = new File(name);
		if ( !file.mkdirs() ) {
			System.out.println("Could not create directory "+name);
		}
	}

	public String getString(String key)
	{
		return key;
	}

	public void setLanguage(LANGUAGE newLanguage)
	{

	}

}
