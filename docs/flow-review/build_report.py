"""Build the local screen inventory, PNG evidence and an offline review gallery."""
from pathlib import Path
import base64
import html
import json
import re
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw, ImageFont

OUT = Path(__file__).resolve().parent
ROOT = OUT.parent.parent
UI = ROOT / 'ui'

GROUPS = [
    ('01 · Versión anterior e introducción nueva', ['frame_1', 'frame_2', 'frame_3', 'intro_1', 'intro_2', 'intro_3', 'intro_5', 'intro_4', 'log_in', 'welcome_back']),
    ('02 · Principal, agenda y formulario', ['today', 'today_c', 'today_b', 'home', 'esta_semana', 'add_task_p1', 'add_task_p2', 'add_task_p3', 'add_task_p4']),
    ('03 · Plan, notificaciones y tarea rápida', ['add_task_p5_d', 'add_task_p5_e', 'add_task_p5', 'add_task_p5_b', 'add_task_p5_c', 'desglose_b', 'desglose', 'add_task_p1_b', 'add_task_p1_c', 'add_task_p1_d']),
]
raw = {p.stem: json.loads(p.read_text(encoding='utf-8')) for p in (UI / '.lvgl-flow/screens').glob('*.json')}
ids = {v['id']: name for name, v in raw.items()}
decisions = {}
for line in (OUT / 'FLOW_REVIEW.md').read_text(encoding='utf-8').splitlines():
    match = re.match(r'\| (\d+) \| `([^`]+)` \| (.*?) \| (.*?) \|$', line)
    if match:
        decisions[match[2]] = {'number': int(match[1]), 'assessment': match[3], 'proposal': match[4]}

inventory = []
for name, source in raw.items():
    original = []
    texts = []

    def visit(node):
        if node.get('type') == 'TEXT':
            texts.append(node.get('characters', ''))
        for reaction in node.get('reactions', []):
            for action in reaction.get('actions', []):
                original.append({'node': node.get('name'), 'trigger': reaction.get('trigger'), 'target': ids.get(action.get('destinationId'), action.get('destinationId')), 'action': action})
        for child in node.get('children', []):
            visit(child)

    visit(source['node'])
    tree = ET.parse(UI / 'screens' / f'{name}.xml')
    routes = [dict(el.attrib) for el in tree.findall('.//screen_create_event')]
    inventory.append({'name': name, 'figma_id': source['id'], 'page': source.get('pageName'), 'text_from_figma': texts, 'figma_reactions': original, 'xml_routes': routes, 'screenshot': f'screens/{name}.png', **decisions[name]})

(OUT / 'inventory.json').write_text(json.dumps({'status': 'source_review_and_static_lvgl_renders', 'screen_count': len(inventory), 'screens': inventory}, ensure_ascii=False, indent=2), encoding='utf-8')
by_name = {item['name']: item for item in inventory}

font = ImageFont.truetype('C:/Windows/Fonts/segoeui.ttf', 16)
for index, (_, names) in enumerate(GROUPS, 1):
    sheet = Image.new('RGB', (1120, ((len(names) + 3) // 4) * 310), '#202128')
    draw = ImageDraw.Draw(sheet)
    for j, name in enumerate(names):
        image = Image.open(OUT / 'screens' / f'{name}.bmp').convert('RGB')
        image.save(OUT / 'screens' / f'{name}.png')
        image.thumbnail((270, 270))
        x, y = (j % 4) * 280 + 5, (j // 4) * 310
        sheet.paste(image, (x, y))
        draw.text((x + 5, y + 277), name, font=font, fill='white')
    sheet.save(OUT / f'overview-{index}.png')

def route(name, screens, events):
    return {'id': name, 'visual_sources': screens, 'events': events}

routes = [
    route('boot', ['intro_1', 'intro_2', 'intro_3', 'intro_5'], {'first_setup': 'onboarding', 'unpaired': 'pair_entry', 'saved_account': 'sync', 'startup_error': 'startup_error'}),
    route('onboarding', ['intro_4'], {'continue': 'pair_entry'}),
    route('pair_entry', ['log_in'], {'pair': 'pairing', 'connections': 'connections'}),
    route('pairing', ['firmware:create_pairing_screen'], {'no_wifi': 'connections', 'verified': 'sync', 'expired_or_failed': 'pairing:error', 'back': 'pair_entry'}),
    route('connections', ['firmware:create_wifi_setup_screen'], {'manual': 'wifi_manual', 'forget': 'wifi_forget_confirm', 'back': '$origin', 'connected': '$resume_pending_operation'}),
    route('wifi_manual', ['firmware:s_manual_wifi_view'], {'back': 'connections'}),
    route('wifi_forget_confirm', ['firmware:s_forget_confirm'], {'confirm': 'connections:provisioning', 'cancel': 'connections'}),
    route('sync', [], {'initial_success': 'welcome', 'background_success': '$current_route', 'offline_with_cache': 'today', 'offline_without_cache': 'startup_error', 'invalid_credentials': 'pair_entry'}),
    route('startup_error', [], {'retry': 'sync', 'connections': 'connections'}),
    route('welcome', ['welcome_back'], {'finished_or_skip': 'today'}),
    route('today', ['today', 'today_c'], {'plus': 'create_menu', 'profile': 'account_settings', 'task_list': 'task_list', 'calendar_or_center': 'agenda_day'}),
    route('account_settings', [], {'connections': 'connections', 'back': 'today', 'unlink_confirmed': '$unlink_service_then_pair_entry'}),
    route('task_list', [], {'select': 'task_detail', 'back': 'today'}),
    route('task_detail', [], {'project_plan': 'project_breakdown', 'back': '$origin'}),
    route('agenda_day', ['home'], {'week': 'agenda_week', 'select_event': 'task_detail', 'select_date': 'date_picker', 'back': 'today'}),
    route('agenda_week', ['esta_semana'], {'day': 'agenda_day', 'select_event': 'task_detail', 'back': 'today'}),
    route('create_menu', ['today_b'], {'project': 'project_identity', 'quick': 'quick_form', 'close': '$origin'}),
    route('project_identity', ['add_task_p1'], {'next_valid': 'project_description', 'cancel_confirmed': '$origin', 'edit_field': 'field_editor'}),
    route('project_description', ['add_task_p2'], {'next_valid': 'project_interval', 'back': 'project_identity', 'edit_field': 'field_editor', 'edit_deadline': 'date_picker'}),
    route('project_interval', ['add_task_p3'], {'next_valid': 'project_generate_confirm', 'back': 'project_description', 'edit_datetime': 'date_picker'}),
    route('project_generate_confirm', ['add_task_p4'], {'generate': 'project_generating', 'back': 'project_interval'}),
    route('project_generating', ['add_task_p5_d'], {'service_success': 'project_summary', 'service_failed': 'project_generation_error', 'cancel': 'project_generate_confirm'}),
    route('project_generation_error', [], {'retry': 'project_generating', 'back': 'project_generate_confirm'}),
    route('project_summary', ['add_task_p5_e'], {'next': 'project_calendar', 'back': 'project_generate_confirm'}),
    route('project_calendar', ['add_task_p5', 'add_task_p5_b', 'add_task_p5_c'], {'change_month': '$update_visible_month', 'next': 'project_breakdown', 'back': 'project_summary', 'select_milestone': 'milestone_editor'}),
    route('project_breakdown', ['desglose_b'], {'next_in_create_mode': 'project_notifications', 'edit': 'milestone_editor', 'back': '$origin_or_project_calendar'}),
    route('project_notifications', ['desglose'], {'save': 'project_saving', 'back': 'project_breakdown', 'edit_preference': 'preference_picker'}),
    route('project_saving', [], {'service_success': 'today', 'service_failed': 'project_save_error'}),
    route('project_save_error', [], {'retry': 'project_saving', 'back': 'project_notifications'}),
    route('quick_form', ['add_task_p1_b'], {'start_valid': 'quick_active', 'cancel': '$origin', 'edit_name': 'field_editor', 'edit_duration': 'duration_picker'}),
    route('quick_active', ['add_task_p1_c'], {'mark_done_and_recorded': 'quick_completed', 'timer_elapsed': '$show_time_finished', 'cancel_confirmed': '$origin', 'leave_view': '$preserve_session_and_return'}),
    route('quick_completed', ['add_task_p1_d'], {'close': '$origin'}),
]
for name in ['date_picker', 'field_editor', 'milestone_editor', 'preference_picker', 'duration_picker']:
    routes.append(route(name, [], {'apply_valid': '$origin_with_updated_model', 'cancel': '$origin'}))
proposal = {'status': 'proposed_not_implemented', 'origin_semantics': 'Return stack; preserve draft, selection and active quick session.', 'route_parameter_requirements': ['origin', 'selected_date', 'task_id', 'project_id', 'draft_id', 'operation_id', 'mode'], 'archived_visual_sources': ['frame_1', 'frame_2', 'frame_3', 'firmware:create_home_placeholder'], 'routes': routes}
assert len(inventory) == 29
route_ids = {r['id'] for r in routes}
for r in routes:
    for target in r['events'].values():
        assert target.startswith('$') or target.split(':')[0] in route_ids, (r['id'], target)
covered = {source for r in routes for source in r['visual_sources']} | set(proposal['archived_visual_sources'])
assert set(raw) <= covered
(OUT / 'proposed-flow.json').write_text(json.dumps(proposal, ensure_ascii=False, indent=2), encoding='utf-8')

parts = ['''<!doctype html><html lang="es"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>HAVIT · Revisión de pantallas y flujo</title><style>
body{margin:0;background:#17191e;color:#eef1f5;font:16px/1.55 system-ui}main{max-width:1500px;margin:auto;padding:32px}h1{font-size:32px}h2{margin-top:44px}a{color:#8fd1f0}.notice{max-width:1000px;color:#c1ccd8}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(320px,1fr));gap:22px}.card{background:#23262e;border:1px solid #404752;border-radius:14px;padding:18px}.card img{display:block;width:100%;max-width:360px;height:auto;margin:auto}.tag{font-size:13px;color:#adbbcc}details{border-top:1px solid #424a58;padding-top:12px}summary{cursor:pointer}li{margin-bottom:10px}code{overflow-wrap:anywhere;font-size:14px}.flow{background:#23262e;border-radius:12px;padding:18px}footer{padding:32px 0;color:#bdc8d5}</style><main>
<h1>HAVIT · Pantallas anteriores y nuevas</h1><p class="notice">29 vistas capturadas con LVGL a 360 × 360 y revisadas individualmente. Esta galería documenta una propuesta de integración; las rutas propuestas aún no están implementadas. Los estados de PIN y conexiones anteriores se conservan por su función y se revisaron desde el código, sin presentar una reconstrucción gráfica como captura real del firmware.</p>
<p><a href="FLOW_REVIEW.md">Análisis completo y flujo recomendado</a> · <a href="inventory.json">Inventario y reacciones originales</a> · <a href="proposed-flow.json">Conexiones propuestas</a></p>
<div class="flow"><strong>Recorrido recomendado</strong><p>Arranque animado → estado de cuenta → presentación/vinculación si hace falta → sincronización → saludo → Hoy.</p><p>Hoy → perfil/ajustes, lista de tareas, agenda diaria/semanal, o + → proyecto/tarea rápida.</p><p>Proyecto → datos → intervalo → generación real → revisión del plan → notificaciones → guardar → Hoy actualizado.</p></div>''']
for title, names in GROUPS:
    parts.append(f'<h2>{html.escape(title)}</h2><div class="grid">')
    for name in names:
        item = by_name[name]
        encoded = base64.b64encode((OUT / 'screens' / f'{name}.png').read_bytes()).decode()
        reactions = ''.join(f'<li>{html.escape(str(r["node"]))}: {html.escape(str(r["trigger"]))} → <code>{html.escape(str(r["target"]))}</code></li>' for r in item['figma_reactions']) or '<li>Sin reacción declarada en la fuente exportada.</li>'
        xml = ', '.join(f'{r.get("trigger")} → {r.get("screen")}' for r in item['xml_routes']) or 'Sin ruta declarada.'
        parts.append(f'<article class="card"><p class="tag">Paso de inventario {item["number"]} · {html.escape(item["page"])}</p><h3>{html.escape(name)}</h3><img alt="Render LVGL de {html.escape(name)}" src="data:image/png;base64,{encoded}"><p>{html.escape(item["assessment"])}</p><p><strong>Propuesta:</strong> {html.escape(item["proposal"])}</p><details><summary>Conexiones encontradas</summary><ul>{reactions}</ul><p><strong>XML:</strong> {html.escape(xml)}</p></details></article>')
    parts.append('</div>')
parts.append('<footer>Los textos y fechas de ejemplo no representan datos de una cuenta real. No se ha verificado interacción en ESP32, red, servicio remoto, entrada de voz, sonido ni iluminación. Usar el informe para distinguir comportamiento existente y propuesto.</footer></main></html>')
(OUT / 'index.html').write_text(''.join(parts), encoding='utf-8')
print(f'Inventario: {len(inventory)} pantallas. Propuesta: {len(routes)} rutas. Cobertura completa de fuentes visuales; destinos internos comprobados.')
