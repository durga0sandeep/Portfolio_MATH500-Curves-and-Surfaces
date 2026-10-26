using UnityEngine;

//
//This is a Singleton Manager Script that is used to communicate between different scripts
public class SingletonScript : MonoBehaviour
{

    public static SingletonScript Instance { get; private set; }

    public int d = 0;

    public CurveSystem curve;

    public Camera Camera;


    //Change to enum later
    // ---> Here, 0 is for BB_Form and 
    // ---> and the 1 ois for NLI_Form
    public int OptionChange = 0; 
    

    private void Awake()
    {
        if (Instance != null && Instance != this)
        {
            Destroy(this);
        }
        else
        {
            Instance = this;
        }
    }


    private void Start()
    {
        d = 1;
        OptionChange = 0;
        UpdatePts();
    }


    public void UpdatePts()
    {
        curve.PointsGenerated(d);
        
    }

}
