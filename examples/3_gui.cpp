#include "bingus.h"

#include <iomanip>
#include <sstream>

#ifdef _DEBUG
#include "LivePP/API/x64/LPP_API_x64_CPP.h"
#endif

void Start();
void Update(float dt);
void Draw();

int main()
{

#ifdef _DEBUG
	// create a default agent, loading the Live++ agent from the given path, e.g. "ThirdParty/LivePP"
	lpp::LppDefaultAgent lppAgent = lpp::LppCreateDefaultAgent(nullptr, L"../include/LivePP");

	// bail out in case the agent is not valid
	if (!lpp::LppIsValidDefaultAgent(&lppAgent))
	{
		return 1;
	}

	// enable Live++ for all loaded modules
	lppAgent.EnableModule(lpp::LppGetCurrentModulePath(), lpp::LPP_MODULES_OPTION_ALL_IMPORT_MODULES, nullptr, nullptr);

	// run the application
	// ...
	//Application::Exec();
#endif


	SetupWindow(1280, 720, "GUI");
	BingusInit();

	SetGameStartFunction(Start);
	SetGameUpdateFunction(Update);
	SetGameDrawFunction(Draw);
	
	RunGame();

#ifdef _DEBUG
	// destroy the Live++ agent
	lpp::LppDestroyDefaultAgent(&lppAgent);
#endif

	return 0;
}

void Start()
{
	globalInputListener.BindAction(KEY_ESCAPE, HOLD, []()
	{
		ExitGame();
	});
}

void Update(float dt)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	GUIContext& gui = globalGUIContext;

	//Styling
	vec4 colorDark = vec4(195.f / 255.f, 165.f / 255.f, 136.f / 255.f, 1);
	vec4 colorMid = vec4(colorDark.x * 1.2f, colorDark.y * 1.2f, colorDark.z * 1.2f, 1);
	vec4 colorLight = vec4(colorDark.x * 1.4f, colorDark.y * 1.4f, colorDark.z * 1.4f, 1);

	gui.defaultImage.color = colorDark;

	gui.defaultLabel.color = colorLight;

	gui.defaultButton.color = colorLight;
	gui.defaultButton.hoverColor = colorMid;
	gui.defaultButton.pressColor = colorDark;

	gui.defaultTickbox.color = colorLight;
	gui.defaultTickbox.hoverColor = colorMid;
	gui.defaultTickbox.pressColor = colorDark;

	gui.defaultSlider.color = colorLight;
	gui.defaultSlider.hoverColor = colorMid;
	gui.defaultSlider.pressColor = colorDark;

	gui.defaultTextField.color = colorLight;

	//GUI
	gui.Image();
		gui.pivot(CENTER);
		gui.anchor(CENTER);
		gui.size(vec2(350, 400));
		gui.source(BOX);
		gui.nineSliceMargin(Edges::All(8.f));

		gui.Column();
			gui.margin(Edges::All(8.f));
			gui.spacing(8.f);

			gui.Label();
				gui.text("Column:");
				gui.margin(Edges::Zero());
				gui.height(40);
				gui.textAlignment(CENTER);
			gui.EndNode();

			gui.LabelButton("Button");
				gui.margin(Edges::Zero());
				gui.height(40);
				gui.nineSliceMargin(Edges::All(8.f));

				gui.onHoverEnter([]() { std::cout << "onHoverEnter\n"; });
				gui.onHover([]() { std::cout << "onHover\n"; });
				gui.onHoverExit([]() { std::cout << "onHoverExit\n"; });
				gui.onPress([]() { std::cout << "onPress\n"; });
				gui.onHold([]() { std::cout << "onHold\n"; });
				gui.onRelease([]() { std::cout << "onRelease\n"; });
			gui.EndNode();

			gui.Widget();
				gui.margin(Edges::Zero());
				gui.height(40);
				gui.Label();
					gui.text("Row:");
					gui.margin(Edges::Zero());
					gui.height(40);
					gui.textAlignment(CENTER_LEFT);
				gui.EndNode();
				gui.Row();
					gui.margin(Edges::Zero());
					gui.anchor(CENTER_RIGHT);
					gui.pivot(CENTER_RIGHT);
					gui.height(40);
					gui.width(185);
					gui.spacing(8.f);

					for (int i = 0; i < 4; i++)
					{
						gui._Button(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + i) + "Button"));
							gui.margin(Edges::Zero());
							gui.width(40);
							//gui.color(colorLight);

							gui._Label(std::hash<std::string>{}(std::string(__FILE__) + std::to_string(__LINE__ + i) + "Label"));
								gui.text(std::to_string(i));
								gui.margin(Edges::Zero());
								gui.textAlignment(CENTER);
							gui.EndNode();
						gui.EndNode();
					}
				gui.EndNode();
			gui.EndNode();

			gui.Widget();
				gui.margin(Edges::Zero());
				gui.height(40);

				gui.Label();
					gui.text("Tickbox: ");
					gui.margin(Edges::Zero());
					gui.height(40);
					gui.textAlignment(CENTER_LEFT);
				gui.EndNode();

				gui.Tickbox();
					gui.anchor(CENTER_RIGHT);
					gui.pivot(CENTER_RIGHT);
					gui.size(vec2(40));
					static bool tickboxVal = false;
					gui.value(&tickboxVal);
				gui.EndNode();
			gui.EndNode();

			gui.Widget();
				gui.margin(Edges::Zero());
				gui.height(40);

				gui.Label();
					gui.text("Slider: ");
					gui.margin(Edges::Zero());
					gui.height(40);
					gui.textAlignment(CENTER_LEFT);
				gui.EndNode();

				gui.Slider();
					gui.anchor(CENTER_RIGHT);
					gui.pivot(CENTER_RIGHT);
					gui.height(40);
					gui.width(180);
					static float sliderVal = 0.f;
					gui.value(&sliderVal);
					gui.min(0.f);
					gui.max(100.f);
				gui.EndNode();
			gui.EndNode();

			gui.Widget();
				gui.margin(Edges::Zero());
				gui.height(180);

				gui.Label();
					gui.text("Text Field: ");
					gui.margin(Edges::Zero());
					gui.height(40);
					gui.textAlignment(CENTER_LEFT);
				gui.EndNode();

				gui.TextField();
					gui.anchor(TOP_RIGHT);
					gui.pivot(TOP_RIGHT);
					gui.height(130);
					gui.width(180);
					gui.text("type here...");
					static std::string textFieldVal = "";
					gui.value(&textFieldVal);
				gui.EndNode();
			gui.EndNode();

			gui.Widget();
				gui.margin(Edges::Zero());
				gui.height(40);

				gui.Label();
					gui.text("Float Field: ");
					gui.margin(Edges::Zero());
					gui.height(40);
					gui.textAlignment(CENTER_LEFT);
				gui.EndNode();

				gui.FloatField();
					gui.anchor(TOP_RIGHT);
					gui.pivot(TOP_RIGHT);
					gui.height(40);
					gui.width(180);
					static float floatFieldVal = 0.f;
					gui.value(&floatFieldVal);
				gui.EndNode();
			gui.EndNode();

		gui.EndNode();
	gui.EndNode();

// 	gui.Image(std::string(__FILE__ ) + std::to_string(__LINE__));
// 		gui.pos(GetWindowSize() / 2.f);
// 		gui.size(vec2(100, 100));
// 		gui.color(vec4(1, 0, 0, 1));
// 	gui.EndNode();

}

void Draw()
{
	
}
