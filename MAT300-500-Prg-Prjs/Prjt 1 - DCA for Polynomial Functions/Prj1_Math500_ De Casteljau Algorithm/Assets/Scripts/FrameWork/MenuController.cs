using UnityEngine;
using TMPro;
public class MenuController : MonoBehaviour
{
    public TMP_Text DegreeTxt;
    public TMP_Dropdown UI_DropDown;
    void Start()
    {
        SingletonScript.Instance.d = 1;
        DegreeTxt.text = SingletonScript.Instance.d.ToString();
    }

    public void d_Increment()
    {
       
        SingletonScript.Instance.d++;
        DegreeTxt.text = SingletonScript.Instance.d.ToString();
    }


    public void d_Decrement()
    {
        if (SingletonScript.Instance.d > 0)
        {
            SingletonScript.Instance.d--;
            DegreeTxt.text = SingletonScript.Instance.d.ToString();
        }
    }

    public void ChangeDegree()
    {
        SingletonScript.Instance.UpdatePts();
    }

    public void DropdownValueChanged(TMP_Dropdown change)
    {
        //0 is for the BB_Form
        //1 for the NLI_Form
        if (change.value == 0)
        {
            SingletonScript.Instance.OptionChange = 0;//BB Form
        }
        else if(change.value == 1)
        {
            SingletonScript.Instance.OptionChange = 1;//NLI Form
        }

    }


}
