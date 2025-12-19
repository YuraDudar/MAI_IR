import streamlit as st
import subprocess
import json
import time
import os
import sys

EXE_PATH = os.path.abspath("../lab_cpp/build/Release/lab4_search.exe")
if sys.platform != "win32":
    EXE_PATH = os.path.abspath("../lab_cpp/build/lab4_search")


CORPUS_DIR = os.path.abspath("../corpus_txt")

def get_engine():
    """Запускает C++ процесс и держит его открытым в session_state"""
    if "engine_process" not in st.session_state:
        if not os.path.exists(EXE_PATH):
            st.error(f"Не найден поисковый движок: {EXE_PATH}")
            return None
        
        exe_dir = os.path.dirname(EXE_PATH)       
        build_dir = os.path.dirname(exe_dir)      
        if sys.platform != "win32":
            build_dir = exe_dir

        try:
            process = subprocess.Popen(
                [EXE_PATH, "--json"],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1, 
                encoding='utf-8',
                cwd=build_dir 
            )
            
            time.sleep(0.5) 
            if process.poll() is not None:
                err = process.stderr.read()
                st.error(f"Движок упал при старте. Ошибка: {err}")
                return None

            st.session_state["engine_process"] = process
            st.toast("Движок подключен!", icon="🚀")
            
        except Exception as e:
            st.error(f"Ошибка запуска: {e}")
            return None
            
    return st.session_state["engine_process"]

def search_in_cpp(query):
    process = get_engine()
    if not process:
        return None

    try:
        process.stdin.write(query + "\n")
        process.stdin.flush()
        
        json_line = process.stdout.readline()
        if not json_line:
            return {"error": "Process returned empty response"}
            
        return json.loads(json_line)
    except Exception as e:
        return {"error": str(e)}

def get_document_content(doc_id):
    """Читает текст файла из папки corpus_txt"""
    filepath = os.path.join(CORPUS_DIR, f"doc_{doc_id}.txt")
    
    if not os.path.exists(filepath):
        return "⚠️ Текст документа не найден на диске (возможно, он не был экспортирован)."
    
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            return f.read()
    except Exception as e:
        return f"Ошибка чтения файла: {e}"


st.set_page_config(page_title="InfoSearch", page_icon="🔍", layout="wide")

st.markdown("""
<style>
    .main-header {font-size: 3rem; color: #4A90E2; text-align: center;}
    .stExpander {border: 1px solid #ddd; border-radius: 8px; margin-bottom: 10px;}
    .doc-meta {color: gray; font-size: 0.8rem; margin-bottom: 10px;}
</style>
""", unsafe_allow_html=True)

if "page" not in st.session_state:
    st.session_state.page = 0
if "results" not in st.session_state:
    st.session_state.results = []
if "last_query" not in st.session_state:
    st.session_state.last_query = ""

st.sidebar.title("Навигация")
mode = st.sidebar.radio("Меню", ["Поиск", "Справка"])

if mode == "Справка":
    st.title("Как пользоваться")
    st.info("Поиск поддерживает сложные булевы запросы.")
    st.markdown("""
    ### Синтаксис:
    - `слово1 слово2` — найдет документы, где есть оба слова (И)
    - `слово1 && слово2` — то же самое (И)
    - `слово1 || слово2` — найдет документы, где есть хотя бы одно (ИЛИ)
    - `!слово` — исключит документы с этим словом (НЕТ)
    - `( ... )` — группировка приоритета
    
    ### Примеры:
    - `(закон || право) && !уголовное`
    - `москва && (метро || транспорт)`
    """)

else:
    st.markdown('<h1 class="main-header">InfoSearch 🔍</h1>', unsafe_allow_html=True)

    col1, col2 = st.columns([4, 1])
    with col1:
        query = st.text_input("Поисковая строка:", value=st.session_state.last_query, placeholder="Введите запрос...")
    with col2:
        st.write("") 
        st.write("") 
        search_btn = st.button("Найти", type="primary", use_container_width=True)

    if search_btn or (query and query != st.session_state.last_query):
        if not query.strip():
            st.warning("Введите запрос!")
        else:
            st.session_state.last_query = query
            st.session_state.page = 0
            
            with st.spinner("Ищем в индексе..."):
                start_time = time.time()
                response = search_in_cpp(query)
                end_time = time.time()
            
            if "error" in response:
                st.error(f"Ошибка поиска: {response['error']}")
                st.session_state.results = []
            else:
                st.session_state.results = response.get("results", [])
                st.session_state.count = response.get("count", 0)
                st.session_state.time_taken = (end_time - start_time) * 1000 

    if st.session_state.results:
        total = st.session_state.count
        timing = st.session_state.time_taken
        
        m1, m2, m3 = st.columns(3)
        m1.metric("Найдено документов", total)
        m2.metric("Время поиска", f"{timing:.2f} ms")
        m3.metric("Всего в базе", "40 823")
        
        st.divider()
        
        RESULTS_PER_PAGE = 20 
        total_pages = (total + RESULTS_PER_PAGE - 1) // RESULTS_PER_PAGE
        
        start_idx = st.session_state.page * RESULTS_PER_PAGE
        end_idx = min(start_idx + RESULTS_PER_PAGE, total)
        
        page_items = st.session_state.results[start_idx:end_idx]
        
        st.caption(f"Показаны результаты {start_idx + 1} - {end_idx}")
        
        for item in page_items:
            doc_id = item['id']
            title = item['title']
            
            with st.expander(f"📄 {title}"):
                st.markdown(f"<div class='doc-meta'>Document ID: {doc_id}</div>", unsafe_allow_html=True)
                
                content = get_document_content(doc_id)
                
                st.text_area("Текст документа:", value=content, height=300, disabled=True, key=f"txt_{doc_id}")
            
        if total_pages > 1:
            st.write("")
            c_prev, c_txt, c_next = st.columns([1, 2, 1])
            
            if c_prev.button("← Назад", disabled=(st.session_state.page == 0)):
                st.session_state.page -= 1
                st.rerun()
                
            c_txt.markdown(f"<div style='text-align:center; padding-top: 5px;'>Страница {st.session_state.page + 1} из {total_pages}</div>", unsafe_allow_html=True)
            
            if c_next.button("Вперед →", disabled=(st.session_state.page >= total_pages - 1)):
                st.session_state.page += 1
                st.rerun()
            
    elif query and search_btn:
        st.info("По вашему запросу ничего не найдено.")